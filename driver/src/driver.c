/*
 * Copyright (C) 2021 Denis Pakhorukov <xpolycone@gmail.com>
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

#include <driver.h>
#include <dsr.h>
#include <context.h>
#include <callback.h>

static PFLT_FILTER Filter = NULL;

NTSTATUS DriverEntry(_In_ PDRIVER_OBJECT DriverObject, _In_ PUNICODE_STRING RegistryPath);
static NTSTATUS DsFilterLoad(_In_ PDRIVER_OBJECT DriverObject, _In_ PUNICODE_STRING RegistryPath);
static NTSTATUS DsFilterUnload(FLT_FILTER_UNLOAD_FLAGS Flags);
static NTSTATUS FLTAPI DsInstanceSetupCallback(
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    FLT_INSTANCE_SETUP_FLAGS Flags,
    DEVICE_TYPE VolumeDeviceType,
    FLT_FILESYSTEM_TYPE VolumeFilesystemType
);
static VOID FLTAPI DsInstanceContextCleanupCallback(_In_ PFLT_CONTEXT Context, FLT_CONTEXT_TYPE ContextType);
static VOID FLTAPI DsStreamContextCleanupCallback(_In_ PFLT_CONTEXT Context, FLT_CONTEXT_TYPE ContextType);
static VOID FLTAPI DsFileContextCleanupCallback(_In_ PFLT_CONTEXT Context, FLT_CONTEXT_TYPE ContextType);

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(INIT, DsFilterLoad)
#pragma alloc_text(PAGE, DsFilterUnload)
#pragma alloc_text(PAGE, DsInstanceSetupCallback)
#pragma alloc_text(PAGE, DsInstanceContextCleanupCallback)
#pragma alloc_text(PAGE, DsStreamContextCleanupCallback)
#pragma alloc_text(PAGE, DsFileContextCleanupCallback)
#endif

static const FLT_CONTEXT_REGISTRATION Contexts[] = {
    { FLT_INSTANCE_CONTEXT, NO_FLAGS, DsInstanceContextCleanupCallback, sizeof(DS_INSTANCE_CONTEXT), DS_DEFAULT_POOL_TAG, NO_CALLBACK, NO_CALLBACK, NULL },
    { FLT_STREAM_CONTEXT, NO_FLAGS, DsStreamContextCleanupCallback, sizeof(DS_STREAM_CONTEXT), DS_DEFAULT_POOL_TAG, NO_CALLBACK, NO_CALLBACK, NULL },
    { FLT_FILE_CONTEXT, NO_FLAGS, DsFileContextCleanupCallback, sizeof(DS_FILE_CONTEXT), DS_DEFAULT_POOL_TAG, NO_CALLBACK, NO_CALLBACK, NULL },
    { FLT_CONTEXT_END }
};

static const FLT_OPERATION_REGISTRATION Callbacks[] = {
    { IRP_MJ_CREATE, NO_FLAGS, DsPreCreateCallback, DsPostCreateCallback, NULL },
    { IRP_MJ_SET_INFORMATION, FLTFL_OPERATION_REGISTRATION_SKIP_PAGING_IO, DsPreSetInformationCallback, DsPostSetInformationCallback, NULL },
    { IRP_MJ_CLEANUP, NO_FLAGS, DsPreCleanupCallback, NO_CALLBACK, NULL },
    { IRP_MJ_OPERATION_END }
};

static const FLT_REGISTRATION FilterRegistration = {
    sizeof(FLT_REGISTRATION),
    FLT_REGISTRATION_VERSION,
    NO_FLAGS,
    Contexts,
    Callbacks,
    DsFilterUnload,
    DsInstanceSetupCallback,
    NO_CALLBACK,
    NO_CALLBACK,
    NO_CALLBACK,
    NO_CALLBACK,
    NO_CALLBACK,
    NO_CALLBACK,
    NO_CALLBACK,
    NO_CALLBACK
};

NTSTATUS DriverEntry(_In_ PDRIVER_OBJECT DriverObject, _In_ PUNICODE_STRING RegistryPath) {
    return DsFilterLoad(DriverObject, RegistryPath);
}

static NTSTATUS DsFilterLoad(_In_ PDRIVER_OBJECT DriverObject, _In_ PUNICODE_STRING RegistryPath) {
    DSR_ENTER(APC_LEVEL);
    DSR_ASSERT(FltRegisterFilter(DriverObject, &FilterRegistration, &Filter));
    DSR_ASSERT(FltStartFiltering(Filter));
    DSR_ERROR_HANDLER({
        if (Filter != NULL)
            FltUnregisterFilter(Filter);
    });
    return DSR_STATUS;
}

static NTSTATUS DsFilterUnload(FLT_FILTER_UNLOAD_FLAGS Flags) {
    DSR_ENTER(PASSIVE_LEVEL);
    FltUnregisterFilter(Filter);
    return DSR_STATUS;
}

static NTSTATUS FLTAPI DsInstanceSetupCallback(
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    FLT_INSTANCE_SETUP_FLAGS Flags,
    DEVICE_TYPE VolumeDeviceType,
    FLT_FILESYSTEM_TYPE VolumeFilesystemType
) {
    DSR_ENTER(PASSIVE_LEVEL);
    PDS_INSTANCE_CONTEXT context = NO_CONTEXT;

#ifdef DBG
    UNICODE_STRING voulmeName = EmptyUnicodeString;
    DsInitUnicodeString(&voulmeName);
    DSR_ASSERT(DsGetVolumeName(FltObjects->Volume, &voulmeName));
    DsLogTrace("Instance setup. [%wZ; 0x%X; 0x%X; 0x%X]", &voulmeName, VolumeDeviceType, VolumeFilesystemType, Flags);
    DsFreeUnicodeString(&voulmeName);
#endif

    if (VolumeDeviceType != FILE_DEVICE_DISK_FILE_SYSTEM)
        DSR_RETURN(STATUS_FLT_DO_NOT_ATTACH);
    if (VolumeFilesystemType == FLT_FSTYPE_RAW)
        DSR_RETURN(STATUS_FLT_DO_NOT_ATTACH);

#if defined(DBG) && defined(ATTACH_VOLUME)
    DECLARE_CONST_UNICODE_STRING(AttachVolume, ATTACH_VOLUME);
    UNICODE_STRING volumeGuid = EmptyUnicodeString;
    DSR_ASSERT(DsGetVolumeGuidName(FltObjects->Volume, &volumeGuid));
    BOOLEAN attach = RtlCompareUnicodeString(&volumeGuid, &AttachVolume, FALSE) == 0;
    DsFreeUnicodeString(&volumeGuid);
    if (!attach)
        DSR_RETURN(STATUS_FLT_DO_NOT_ATTACH);
#endif

    DSR_ASSERT(FltAllocateContext(FltObjects->Filter, FLT_INSTANCE_CONTEXT, sizeof(DS_INSTANCE_CONTEXT), PagedPool, &context));
    DSR_ASSERT(DsInitializeInstanceContext(FltObjects, context));
    DSR_ASSERT(FltSetInstanceContext(FltObjects->Instance, FLT_SET_CONTEXT_KEEP_IF_EXISTS, context, NULL));

    DsLogInfo("Instance context created. [0x%p]", context);

    DSR_ERROR_HANDLER({});

    FltReleaseContextSafe(context);
    return DSR_STATUS;
}

static VOID FLTAPI DsInstanceContextCleanupCallback(_In_ PFLT_CONTEXT Context, FLT_CONTEXT_TYPE ContextType) {
    PDS_INSTANCE_CONTEXT context = (PDS_INSTANCE_CONTEXT)Context;
    DsLogTrace("Instance context cleanup. [%wZ]", &context->VolumeGuid);
    DsFinalizeInstanceContext(context);
}

static VOID FLTAPI DsFileContextCleanupCallback(_In_ PFLT_CONTEXT Context, FLT_CONTEXT_TYPE ContextType) {
    PDS_FILE_CONTEXT context = (PDS_FILE_CONTEXT)Context;
    DsLogTrace("File context cleanup. [0x%p]", context);
    DsFinalizeFileContext(context);
}

static VOID FLTAPI DsStreamContextCleanupCallback(_In_ PFLT_CONTEXT Context, FLT_CONTEXT_TYPE ContextType) {
    PDS_STREAM_CONTEXT context = (PDS_STREAM_CONTEXT)Context;
#ifdef DBG
    DsLogTrace("Stream context cleanup. [0x%p; %wZ]", context, &context->Name);
#endif
    DsFinalizeStreamContext(context);
}
