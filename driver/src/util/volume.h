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

#pragma once
#include "common.h"

typedef struct _DS_VOLUME_PROPERTIES {
    DEVICE_TYPE DeviceType;
    ULONG DeviceObjectFlags;
    ULONG DeviceCharacteristics;
    USHORT SectorSize;
} DS_VOLUME_PROPERTIES, *PDS_VOLUME_PROPERTIES;

NTSTATUS DsGetVolumeGuidName(_In_ PFLT_VOLUME Volume, _Inout_ PUNICODE_STRING Name);
NTSTATUS DsGetVolumeProperties(_In_ PFLT_VOLUME Volume, _Inout_ PDS_VOLUME_PROPERTIES VolumeProperties);
