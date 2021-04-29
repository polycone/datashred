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

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, DsGetVolumeGuidName)
#pragma alloc_text(PAGE, DsGetVolumeProperties)
#pragma alloc_text(PAGE, DsGetFileSystemProperties)
#endif

#pragma warning(push)
#pragma warning(disable:4242 4244) // Possible data loss within conversion

NTSTATUS DsGetVolumeGuidName(_In_ PFLT_VOLUME Volume, _Inout_ PUNICODE_STRING Name) {
    DSR_INIT(PASSIVE_LEVEL);
    ULONG bufferLength = 0;
    DSR_ASSERT(FltGetVolumeGuidName(Volume, NULL, &bufferLength), DSR_SUPPRESS(STATUS_BUFFER_TOO_SMALL));
    if (Name->MaximumLength < bufferLength) {
        DsFreeUnicodeString(Name);
        DSR_ASSERT(DsCreateUnicodeString(Name, bufferLength));
    }
    DSR_ASSERT(FltGetVolumeGuidName(Volume, Name, &bufferLength));
    DSR_CLEANUP {
        DsFreeUnicodeString(Name);
    }
    return DSR_STATUS;
}

#pragma warning(pop)

NTSTATUS DsGetVolumeProperties(_In_ PFLT_VOLUME Volume, _Out_ PDS_VOLUME_PROPERTIES Properties) {
    DSR_INIT(APC_LEVEL);
    ULONG bufferLength = 0;
    FLT_VOLUME_PROPERTIES properties;
    DSR_ASSERT(
        FltGetVolumeProperties(Volume, &properties, sizeof(FLT_VOLUME_PROPERTIES), &bufferLength),
        DSR_SUPPRESS(STATUS_BUFFER_OVERFLOW)
    );
    Properties->SectorSize = properties.SectorSize;
    DSR_CLEANUP { }
    return DSR_STATUS;
}

NTSTATUS DsGetFileSystemProperties(_In_ PFLT_INSTANCE Instance, _Out_ PDS_FILESYSTEM_PROPERTIES Properties) {
    DSR_INIT(PASSIVE_LEVEL);
    IO_STATUS_BLOCK ioStatus;
    FILE_FS_ATTRIBUTE_INFORMATION fsAttributeInfo;
    DSR_ASSERT(
        FltQueryVolumeInformation(Instance, &ioStatus, &fsAttributeInfo, sizeof(FILE_FS_ATTRIBUTE_INFORMATION), FileFsAttributeInformation),
        DSR_SUPPRESS(STATUS_BUFFER_OVERFLOW)
    );
    DSR_ASSERT(FltGetFileSystemType(Instance, &Properties->Type));
    Properties->Attributes = fsAttributeInfo.FileSystemAttributes;
    DSR_CLEANUP { }
    return DSR_STATUS;
}
