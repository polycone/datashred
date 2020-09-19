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
#include "util/string.h"
#include "util/memory.h"

NTSTATUS DriverEntry(_In_ PDRIVER_OBJECT driverObject, _In_ PUNICODE_STRING pRegistryPath);
static NTSTATUS DsFilterLoad(_In_ PDRIVER_OBJECT driverObject, _In_ PUNICODE_STRING pRegistryPath);
static NTSTATUS DsFilterUnload(_In_ FLT_FILTER_UNLOAD_FLAGS flags);

static NTSTATUS FLTAPI DsInstanceSetupCallback(
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ FLT_INSTANCE_SETUP_FLAGS Flags,
    _In_ DEVICE_TYPE VolumeDeviceType,
    _In_ FLT_FILESYSTEM_TYPE VolumeFilesystemType
);

static NTSTATUS GetVolumeGuidName(_In_ PFLT_VOLUME Volume, _Inout_ PUNICODE_STRING *String);

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(INIT, DsFilterLoad)
#pragma alloc_text(PAGE, DsFilterUnload)
#pragma alloc_text(PAGE, DsInstanceSetupCallback)
#endif

static const FLT_OPERATION_REGISTRATION callbacks[] = {
    { IRP_MJ_OPERATION_END }
};

static const FLT_REGISTRATION filterRegistration = {
    sizeof(FLT_REGISTRATION),
    FLT_REGISTRATION_VERSION,
    FLTFL_NONE,
    NO_CONTEXT,
    callbacks,
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
    PAGED_CODE();
    NTSTATUS status = STATUS_SUCCESS;
    if (VolumeDeviceType != FILE_DEVICE_DISK && VolumeDeviceType != FILE_DEVICE_DISK_FILE_SYSTEM)
        return STATUS_FLT_DO_NOT_ATTACH;
    if (VolumeFilesystemType == FLT_FSTYPE_RAW)
        return STATUS_FLT_DO_NOT_ATTACH;
    PUNICODE_STRING name = NULL;
    status = GetVolumeGuidName(FltObjects->Volume, &name);
    if (!NT_SUCCESS(status))
        return status;
    DsLogInfo("Attached to: %wZ", name);
    DsMemFree(name);
    return status;
}

static NTSTATUS GetVolumeGuidName(_In_ PFLT_VOLUME Volume, _Inout_ PUNICODE_STRING *Name) {
    ULONG bufferLength = 0;
    NTSTATUS status = FltGetVolumeGuidName(Volume, NULL, &bufferLength);
    if (status != STATUS_BUFFER_TOO_SMALL && !NT_SUCCESS(status))
        return status;
    PUNICODE_STRING name = NULL;
    status = DsAllocUnicodeString((USHORT)bufferLength, &name);
    if (!NT_SUCCESS(status))
        return status;
    status = FltGetVolumeGuidName(Volume, name, &bufferLength);
    if (NT_SUCCESS(status)) {
        *Name = name;
        return status;
    }
    DsMemFree(name);
    return status;
}
