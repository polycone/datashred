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

#include "driver.h"
#include "dsr.h"
#include "volume.h"

typedef NTSTATUS
(FLTAPI *PDS_GET_VOLUME_NAME_ROUTINE) (
    _In_ PFLT_VOLUME Volume,
    _Inout_opt_ PUNICODE_STRING VolumeName,
    _Out_opt_ PULONG BufferSizeNeeded
);
static NTSTATUS DsGetVoulmeNameBy(_In_ PDS_GET_VOLUME_NAME_ROUTINE Getter, _In_ PFLT_VOLUME Volume, _Inout_ PUNICODE_STRING Name);

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, DsGetVolumeGuidName)
#pragma alloc_text(PAGE, DsGetVolumeName)
#pragma alloc_text(PAGE, DsGetVolumeProperties)
#pragma alloc_text(PAGE, DsGetFileSystemProperties)
#endif

NTSTATUS DsGetVolumeGuidName(_In_ PFLT_VOLUME Volume, _Inout_ PUNICODE_STRING Name) {
    return DsGetVoulmeNameBy(FltGetVolumeGuidName, Volume, Name);
}

NTSTATUS DsGetVolumeName(_In_ PFLT_VOLUME Volume, _Inout_ PUNICODE_STRING Name) {
    return DsGetVoulmeNameBy(FltGetVolumeName, Volume, Name);
}

#pragma warning(push)
#pragma warning(disable:4242 4244) // Possible data loss within conversion

static NTSTATUS DsGetVoulmeNameBy(_In_ PDS_GET_VOLUME_NAME_ROUTINE Getter, _In_ PFLT_VOLUME Volume, _Inout_ PUNICODE_STRING Name) {
    DSR_ENTER(PASSIVE_LEVEL);
    ULONG bufferLength = 0;
    DSR_ASSERT(Getter(Volume, NULL, &bufferLength), DSR_SUPPRESS(STATUS_BUFFER_TOO_SMALL));
    if (Name->MaximumLength < bufferLength) {
        DsFreeUnicodeString(Name);
        DSR_ASSERT(DsAllocateUnicodeString(Name, bufferLength));
    }
    DSR_ASSERT(Getter(Volume, Name, &bufferLength));
    DSR_ERROR_HANDLER({
        DsFreeUnicodeString(Name);
    });
    return DSR_STATUS;
}

#pragma warning(pop)

NTSTATUS DsGetVolumeProperties(_In_ PFLT_VOLUME Volume, _Out_ PDS_VOLUME_PROPERTIES Properties) {
    DSR_ENTER(APC_LEVEL);
    ULONG bufferLength = 0;
    FLT_VOLUME_PROPERTIES properties;
    DSR_ASSERT(
        FltGetVolumeProperties(Volume, &properties, sizeof(FLT_VOLUME_PROPERTIES), &bufferLength),
        DSR_SUPPRESS(STATUS_BUFFER_OVERFLOW)
    );
    Properties->SectorSize = properties.SectorSize;
    DSR_ERROR_HANDLER({});
    return DSR_STATUS;
}

NTSTATUS DsGetFileSystemProperties(_In_ PFLT_INSTANCE Instance, _Out_ PDS_FILESYSTEM_PROPERTIES Properties) {
    DSR_ENTER(PASSIVE_LEVEL);
    IO_STATUS_BLOCK ioStatus;
    FILE_FS_ATTRIBUTE_INFORMATION fsAttributeInfo;
    DSR_ASSERT(
        FltQueryVolumeInformation(Instance, &ioStatus, &fsAttributeInfo, sizeof(FILE_FS_ATTRIBUTE_INFORMATION), FileFsAttributeInformation),
        DSR_SUPPRESS(STATUS_BUFFER_OVERFLOW)
    );
    DSR_ASSERT(FltGetFileSystemType(Instance, &Properties->Type));
    Properties->Attributes = fsAttributeInfo.FileSystemAttributes;
    DSR_ERROR_HANDLER({});
    return DSR_STATUS;
}
