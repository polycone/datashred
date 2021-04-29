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
#include <context.h>
#include <callback.h>

PFLT_FILTER Filter = NULL;

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
static VOID FLTAPI DsStreamContextCleanupCallback(_In_ PFLT_CONTEXT Context, _In_ FLT_CONTEXT_TYPE ContextType);
static VOID FLTAPI DsFileContextCleanupCallback(_In_ PFLT_CONTEXT Context, _In_ FLT_CONTEXT_TYPE ContextType);

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(INIT, DsFilterLoad)
#pragma alloc_text(PAGE, DsFilterUnload)
#pragma alloc_text(PAGE, DsInstanceSetupCallback)
#pragma alloc_text(PAGE, DsInstanceContextCleanupCallback)
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
    DSR_INIT(APC_LEVEL);
    DSR_ASSERT(FltRegisterFilter(DriverObject, &FilterRegistration, &Filter));
    DSR_ASSERT(FltStartFiltering(Filter));
    DSR_CLEANUP {
        DSR_LOG_UNEXPECTED_ERROR();
        if (Filter != NULL)
            FltUnregisterFilter(Filter);
    }
    return DSR_STATUS;
}

static NTSTATUS DsFilterUnload(FLT_FILTER_UNLOAD_FLAGS Flags) {
    DSR_INIT(PASSIVE_LEVEL);
    FltUnregisterFilter(Filter);
    return DSR_STATUS;
}

static NTSTATUS FLTAPI DsInstanceSetupCallback(
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    FLT_INSTANCE_SETUP_FLAGS Flags,
    DEVICE_TYPE VolumeDeviceType,
    FLT_FILESYSTEM_TYPE VolumeFilesystemType
) {
    DSR_INIT(PASSIVE_LEVEL);
    DsLogTrace(
        "Trying to setup an instance. DE: 0x%08X, FS: 0x%08X, FL: 0x%08X.",
        VolumeDeviceType, VolumeFilesystemType, Flags
    );
    if (VolumeDeviceType != FILE_DEVICE_DISK_FILE_SYSTEM)
        return STATUS_FLT_DO_NOT_ATTACH;
    if (VolumeFilesystemType == FLT_FSTYPE_RAW)
        return STATUS_FLT_DO_NOT_ATTACH;

    PDS_INSTANCE_CONTEXT context = NO_CONTEXT;

#ifdef ATTACH_VOLUME
    DECLARE_CONST_UNICODE_STRING(AttachVolume, ATTACH_VOLUME);
    UNICODE_STRING guid = EmptyUnicodeString;
    DsInitUnicodeString(&guid);
    DSR_ASSERT(DsGetVolumeGuidName(FltObjects->Volume, &guid));
    BOOLEAN attach = RtlCompareUnicodeString(&guid, &AttachVolume, FALSE) == 0;
    DsFreeUnicodeString(&guid);
    if (!attach)
        return STATUS_FLT_DO_NOT_ATTACH;
#endif

    DSR_ASSERT(FltAllocateContext(FltObjects->Filter, FLT_INSTANCE_CONTEXT, sizeof(DS_INSTANCE_CONTEXT), PagedPool, &context));
    DSR_ASSERT(DsInitInstanceContext(FltObjects, context));
    DSR_ASSERT(FltSetInstanceContext(FltObjects->Instance, FLT_SET_CONTEXT_KEEP_IF_EXISTS, context, NULL));

    DsLogInfo(
        "Instance context created.\n"
        "  - Volume: %wZ\n"
        "  - File system: %d\n"
        "  - File system attributes: 0x%08X\n"
        "  - Sector size: %d\n",
        &context->VolumeGuid,
        context->FileSystemProperties.Type,
        context->FileSystemProperties.Attributes,
        context->VolumeProperties.SectorSize
    );

    DSR_CLEANUP { }
    FltReleaseContextSafe(context);
    return DSR_STATUS;
}

static VOID FLTAPI DsInstanceContextCleanupCallback(_In_ PFLT_CONTEXT Context, FLT_CONTEXT_TYPE ContextType) {
    PDS_INSTANCE_CONTEXT context = (PDS_INSTANCE_CONTEXT)Context;
    DsLogTrace(
        "Instance context is being cleaned up.\n"
        "  - Volume: %wZ.",
        &context->VolumeGuid
    );
    DsFreeInstanceContext(context);
}

static VOID FLTAPI DsFileContextCleanupCallback(_In_ PFLT_CONTEXT Context, _In_ FLT_CONTEXT_TYPE ContextType) {
    PDS_FILE_CONTEXT context = (PDS_FILE_CONTEXT)Context;
    DsFreeFileContext(context);
}

static VOID FLTAPI DsStreamContextCleanupCallback(_In_ PFLT_CONTEXT Context, _In_ FLT_CONTEXT_TYPE ContextType) {
    PDS_STREAM_CONTEXT context = (PDS_STREAM_CONTEXT)Context;
    DsFreeStreamContext(context);
}
