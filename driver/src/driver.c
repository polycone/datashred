/*
 * Copyright (C) 2020 Denis Pakhorukov <xpolycone@gmail.com>
 *
 * This file is part of Datashred.
 *
 * Datashred is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3
 * as published by the Free Software Foundation.
 *
 * Datashred is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Datashred. If not, see <https://www.gnu.org/licenses/>.
 */

#include "common.h"
#include "util/volume.h"
#include "util/memory.h"
#include "util/string.h"
#include "context/instance.h"
#include "context/stream.h"
#include "callback.h"

PFLT_FILTER Filter = NULL;

NTSTATUS DriverEntry(_In_ PDRIVER_OBJECT driverObject, _In_ PUNICODE_STRING pRegistryPath);
static NTSTATUS DsFilterLoad(_In_ PDRIVER_OBJECT driverObject, _In_ PUNICODE_STRING pRegistryPath);
static NTSTATUS DsFilterUnload(_In_ FLT_FILTER_UNLOAD_FLAGS flags);

static NTSTATUS FLTAPI DsInstanceSetupCallback(
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ FLT_INSTANCE_SETUP_FLAGS Flags,
    _In_ DEVICE_TYPE VolumeDeviceType,
    _In_ FLT_FILESYSTEM_TYPE VolumeFilesystemType
);

static VOID FLTAPI DsInstanceContextCleanupCallback(_In_ PFLT_CONTEXT Context, _In_ FLT_CONTEXT_TYPE ContextType);
static VOID FLTAPI DsStreamContextCleanupCallback(_In_ PFLT_CONTEXT Context, _In_ FLT_CONTEXT_TYPE ContextType);

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(INIT, DsFilterLoad)
#pragma alloc_text(PAGE, DsFilterUnload)
#pragma alloc_text(PAGE, DsInstanceSetupCallback)
#pragma alloc_text(PAGE, DsInstanceContextCleanupCallback)
#pragma alloc_text(PAGE, DsStreamContextCleanupCallback)
#endif

static const FLT_CONTEXT_REGISTRATION contexts[] = {
    { FLT_INSTANCE_CONTEXT, EMPTY_FLAGS, DsInstanceContextCleanupCallback, sizeof(DS_INSTANCE_CONTEXT), DS_DEFAULT_POOL_TAG, EMPTY_CALLBACK, EMPTY_CALLBACK, NULL },
    { FLT_STREAM_CONTEXT, EMPTY_FLAGS, DsStreamContextCleanupCallback, sizeof(DS_STREAM_CONTEXT), DS_DEFAULT_POOL_TAG, EMPTY_CALLBACK, EMPTY_CALLBACK, NULL },
    { FLT_CONTEXT_END }
};

static const FLT_OPERATION_REGISTRATION callbacks[] = {
    { IRP_MJ_CREATE, EMPTY_FLAGS, DsPreCreateCallback, DsPostCreateCallback, NULL },
    { IRP_MJ_OPERATION_END }
};

static const FLT_REGISTRATION filterRegistration = {
    sizeof(FLT_REGISTRATION),
    FLT_REGISTRATION_VERSION,
    EMPTY_FLAGS,
    contexts,
    callbacks,
    DsFilterUnload,
    DsInstanceSetupCallback,
    EMPTY_CALLBACK,
    EMPTY_CALLBACK,
    EMPTY_CALLBACK,
    EMPTY_CALLBACK,
    EMPTY_CALLBACK,
    EMPTY_CALLBACK,
    EMPTY_CALLBACK,
    EMPTY_CALLBACK
};

NTSTATUS DriverEntry(_In_ PDRIVER_OBJECT pDriverObject, _In_ PUNICODE_STRING pRegistryPath) {
    return DsFilterLoad(pDriverObject, pRegistryPath);
}

static NTSTATUS DsFilterLoad(_In_ PDRIVER_OBJECT pDriverObject, _In_ PUNICODE_STRING pRegistryPath) {
    UNREFERENCED_PARAMETER(pRegistryPath);
    DSR_INIT(APC_LEVEL);
    DSR_ASSERT(FltRegisterFilter(pDriverObject, &filterRegistration, &Filter));
    DSR_ASSERT(FltStartFiltering(Filter));
    DSR_CLEANUP_START();
    if (Filter != NULL) {
        FltUnregisterFilter(Filter);
    }
    DSR_CLEANUP_END();
    return DSR_STATUS;
}

static NTSTATUS DsFilterUnload(_In_ FLT_FILTER_UNLOAD_FLAGS Flags) {
    UNREFERENCED_PARAMETER(Flags);
    DSR_INIT(APC_LEVEL);
    FltUnregisterFilter(Filter);
    return DSR_STATUS;
}

static NTSTATUS FLTAPI DsInstanceSetupCallback(
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ FLT_INSTANCE_SETUP_FLAGS Flags,
    _In_ DEVICE_TYPE VolumeDeviceType,
    _In_ FLT_FILESYSTEM_TYPE VolumeFilesystemType
) {
    UNREFERENCED_PARAMETER(Flags);
    DSR_INIT(PASSIVE_LEVEL);
    DsLogTrace(
        "Trying to setup an instance. DE: 0x%08X, FS: 0x%08X, FL: 0x%08X.",
        VolumeDeviceType, VolumeFilesystemType, Flags
    );

    if (VolumeDeviceType != FILE_DEVICE_DISK_FILE_SYSTEM)
        return STATUS_FLT_DO_NOT_ATTACH;
    if (VolumeFilesystemType == FLT_FSTYPE_RAW)
        return STATUS_FLT_DO_NOT_ATTACH;

    PDS_INSTANCE_CONTEXT context = EMPTY_CONTEXT;
    DSR_ASSERT(FltAllocateContext(FltObjects->Filter, FLT_INSTANCE_CONTEXT, sizeof(DS_INSTANCE_CONTEXT), PagedPool, &context));
    DSR_ASSERT(DsInitInstanceContext(FltObjects, context));
    DSR_ASSERT(FltSetInstanceContext(FltObjects->Instance, FLT_SET_CONTEXT_KEEP_IF_EXISTS, context, NULL));

    DsLogInfo(
        "Instance context created.\n"
        "  - Volume: %wZ\n"
        "  - Filesystem: %d\n"
        "  - Sector size: %d",
        &context->VolumeGuid,
        context->VolumeProperties.FilesystemType,
        context->VolumeProperties.SectorSize
    );

    DSR_CLEANUP_EMPTY();
    if (context != EMPTY_CONTEXT) {
        FltReleaseContext(context);
    }
    return DSR_STATUS;
}

static VOID FLTAPI DsInstanceContextCleanupCallback(_In_ PFLT_CONTEXT Context, _In_ FLT_CONTEXT_TYPE ContextType) {
    UNREFERENCED_PARAMETER(ContextType);
    PDS_INSTANCE_CONTEXT context = (PDS_INSTANCE_CONTEXT)Context;
    DsLogInfo(
        "Instance context is being cleaned up.\n"
        "  - Volume: %wZ.",
        &context->VolumeGuid
    );
    DsFreeInstanceContext(context);
}

static VOID FLTAPI DsStreamContextCleanupCallback(_In_ PFLT_CONTEXT Context, _In_ FLT_CONTEXT_TYPE ContextType) {
    UNREFERENCED_PARAMETER(ContextType);
    PDS_STREAM_CONTEXT context = (PDS_STREAM_CONTEXT)Context;
    DsLogTrace("Stream context is being cleaned up. File name: %wZ.", &context->FileName);
    DsFreeStreamContext(context);
}
