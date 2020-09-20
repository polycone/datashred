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
#include "filter.h"
#include "util/volume.h"
#include "util/memory.h"
#include "util/string.h"
#include "context.h"

NTSTATUS DriverEntry(_In_ PDRIVER_OBJECT driverObject, _In_ PUNICODE_STRING pRegistryPath);
static NTSTATUS DsFilterLoad(_In_ PDRIVER_OBJECT driverObject, _In_ PUNICODE_STRING pRegistryPath);
static NTSTATUS DsFilterUnload(_In_ FLT_FILTER_UNLOAD_FLAGS flags);

static NTSTATUS FLTAPI DsInstanceSetupCallback(
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ FLT_INSTANCE_SETUP_FLAGS Flags,
    _In_ DEVICE_TYPE VolumeDeviceType,
    _In_ FLT_FILESYSTEM_TYPE VolumeFilesystemType
);

static VOID FLTAPI DsContextCleanupCallback(_In_ PFLT_CONTEXT Context, _In_ FLT_CONTEXT_TYPE ContextType);
static NTSTATUS DsInitInstanceContext(_In_ PCFLT_RELATED_OBJECTS FltObjects, _Inout_ PDS_INSTANCE_CONTEXT *Context);
static VOID DsFreeInstanceContext(_In_ PDS_INSTANCE_CONTEXT Context);

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(INIT, DsFilterLoad)
#pragma alloc_text(PAGE, DsFilterUnload)
#pragma alloc_text(PAGE, DsInstanceSetupCallback)
#pragma alloc_text(PAGE, DsContextCleanupCallback)
#endif

static const FLT_CONTEXT_REGISTRATION contexts[] = {
    { FLT_INSTANCE_CONTEXT, EMPTY_FLAGS, DsContextCleanupCallback, sizeof(DS_INSTANCE_CONTEXT), DS_DEFAULT_POOL_TAG, EMPTY_CALLBACK, EMPTY_CALLBACK, NULL },
    { IRP_MJ_OPERATION_END }
};

static const FLT_OPERATION_REGISTRATION callbacks[] = {
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
    NTSTATUS status = FltRegisterFilter(pDriverObject, &filterRegistration, &Filter);
    if (!NT_SUCCESS(status)) {
        return status;
    }
    status = FltStartFiltering(Filter);
    if (!NT_SUCCESS(status)) {
        FltUnregisterFilter(Filter);
    }
    return status;
}

static NTSTATUS DsFilterUnload(_In_ FLT_FILTER_UNLOAD_FLAGS Flags) {
    UNREFERENCED_PARAMETER(Flags);
    PAGED_CODE();
    FltUnregisterFilter(Filter);
    return STATUS_SUCCESS;
}

static NTSTATUS FLTAPI DsInstanceSetupCallback(
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ FLT_INSTANCE_SETUP_FLAGS Flags,
    _In_ DEVICE_TYPE VolumeDeviceType,
    _In_ FLT_FILESYSTEM_TYPE VolumeFilesystemType
) {
    UNREFERENCED_PARAMETER(Flags);
    DSR_INIT;

    if (VolumeDeviceType != FILE_DEVICE_DISK && VolumeDeviceType != FILE_DEVICE_DISK_FILE_SYSTEM)
        return STATUS_FLT_DO_NOT_ATTACH;
    if (VolumeFilesystemType == FLT_FSTYPE_RAW)
        return STATUS_FLT_DO_NOT_ATTACH;

    PDS_INSTANCE_CONTEXT context = EMPTY_CONTEXT;
    DSR_ASSERT(DsInitInstanceContext(FltObjects, &context));

    DSR_CLEANUP {
        DsFreeUnicodeString(&context->VolumeGuid);
    };
    if (context != EMPTY_CONTEXT) {
        FltReleaseContext(context);
    }
    return DSR_STATUS;
}

static VOID FLTAPI DsContextCleanupCallback(_In_ PFLT_CONTEXT Context, _In_ FLT_CONTEXT_TYPE ContextType) {
    switch (ContextType) {
        case FLT_INSTANCE_CONTEXT:
            DsFreeInstanceContext((PDS_INSTANCE_CONTEXT)Context);
            break;
    }
}

static NTSTATUS DsInitInstanceContext(_In_ PCFLT_RELATED_OBJECTS FltObjects, _Inout_ PDS_INSTANCE_CONTEXT *Context) {
    DSR_INIT;
    PDS_INSTANCE_CONTEXT context = EMPTY_CONTEXT;
    DSR_ASSERT(FltAllocateContext(Filter, FLT_INSTANCE_CONTEXT, sizeof(DS_INSTANCE_CONTEXT), PagedPool, &context));
    DsInitUnicodeString(&context->VolumeGuid);
    DSR_ASSERT(DsGetVolumeGuidName(FltObjects->Volume, &context->VolumeGuid));
    DSR_ASSERT(FltSetInstanceContext(FltObjects->Instance, FLT_SET_CONTEXT_KEEP_IF_EXISTS, context, NULL));
    *Context = context;
    DsLogInfo("Instance context created. Volume: %wZ.", &context->VolumeGuid);
    DSR_CLEANUP {
        DsFreeInstanceContext(context);
    };
    return DSR_STATUS;
}

static VOID DsFreeInstanceContext(_In_ PDS_INSTANCE_CONTEXT Context) {
    DsLogInfo("Instance context is being cleaned up. Volume: %wZ.", &Context->VolumeGuid);
    DsFreeUnicodeString(&Context->VolumeGuid);
}
