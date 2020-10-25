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

#include "volume.h"
#include "string.h"
#include "memory.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, DsGetVolumeGuidName)
#pragma alloc_text(PAGE, DsGetVolumeProperties)
#pragma alloc_text(PAGE, DsGetFileSystemProperties)
#endif

NTSTATUS DsGetVolumeGuidName(_In_ PFLT_VOLUME Volume, _Out_ PUNICODE_STRING Name) {
    DSR_INIT(PASSIVE_LEVEL);
    ULONG bufferLength = 0;
    DSR_ASSERT(FltGetVolumeGuidName(Volume, NULL, &bufferLength), DSR_SUPPRESS(STATUS_BUFFER_TOO_SMALL));
    DSR_ASSERT(DsEnsureUnicodeStringLength(Name, (USHORT)bufferLength));
    DSR_ASSERT(FltGetVolumeGuidName(Volume, Name, &bufferLength));
    DSR_CLEANUP_START();
    DsFreeUnicodeString(Name);
    DSR_CLEANUP_END();
    return DSR_STATUS;
}

NTSTATUS DsGetVolumeProperties(_In_ PFLT_VOLUME Volume, _Out_ PDS_VOLUME_PROPERTIES Properties) {
    DSR_INIT(APC_LEVEL);
    ULONG bufferLength = 0;
    FLT_VOLUME_PROPERTIES properties;
    DSR_ASSERT(
        FltGetVolumeProperties(Volume, &properties, sizeof(FLT_VOLUME_PROPERTIES), &bufferLength),
        DSR_SUPPRESS(STATUS_BUFFER_OVERFLOW)
    );
    Properties->SectorSize = properties.SectorSize;
    DSR_CLEANUP_EMPTY();
    return DSR_STATUS;
}

#define FS_ATTR_INFO_SIZE sizeof(FILE_FS_ATTRIBUTE_INFORMATION)

NTSTATUS DsGetFileSystemProperties(_In_ PFLT_INSTANCE Instance, _Out_ PDS_FILESYSTEM_PROPERTIES Properties) {
    DSR_INIT(PASSIVE_LEVEL);
    IO_STATUS_BLOCK ioStatus;
    FILE_FS_ATTRIBUTE_INFORMATION fsAttributeInfo;
    DSR_ASSERT(
        FltQueryVolumeInformation(Instance, &ioStatus, &fsAttributeInfo, FS_ATTR_INFO_SIZE, FileFsAttributeInformation),
        DSR_SUPPRESS(STATUS_BUFFER_OVERFLOW)
    );
    DSR_ASSERT(FltGetFileSystemType(Instance, &Properties->Type));
    Properties->Attributes = fsAttributeInfo.FileSystemAttributes;
    DSR_CLEANUP_EMPTY();
    return DSR_STATUS;
}
