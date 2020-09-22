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
#endif

NTSTATUS DsGetVolumeGuidName(_In_ PFLT_VOLUME Volume, _Inout_ PUNICODE_STRING Name) {
    ULONG bufferLength = 0;
    NTSTATUS status = FltGetVolumeGuidName(Volume, NULL, &bufferLength);
    if (status != STATUS_BUFFER_TOO_SMALL && !NT_SUCCESS(status)) {
        return status;
    }
    status = DsEnsureUnicodeStringLength(Name, (USHORT)bufferLength);
    if (!NT_SUCCESS(status)) {
        return status;
    }
    status = FltGetVolumeGuidName(Volume, Name, &bufferLength);
    if (!NT_SUCCESS(status)) {
        DsMemFree(Name);
    }
    return status;
}

NTSTATUS DsGetVolumeProperties(_In_ PFLT_VOLUME Volume, _Inout_ PDS_VOLUME_PROPERTIES VolumeProperties) {
    DSR_INIT;
    ULONG bufferLength = 0;
    FLT_VOLUME_PROPERTIES properties;
    DSR_ASSERT(
        FltGetVolumeProperties(Volume, &properties, sizeof(FLT_VOLUME_PROPERTIES), &bufferLength),
        DSR_SUPPRESS(STATUS_BUFFER_OVERFLOW)
    );
    VolumeProperties->DeviceType = properties.DeviceType;
    VolumeProperties->DeviceCharacteristics = properties.DeviceCharacteristics;
    VolumeProperties->SectorSize = properties.SectorSize;
    DSR_CLEANUP;
    return DSR_STATUS;
}
