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

#pragma once
#include <driver.h>

typedef struct _DS_INSTANCE_CONTEXT {
    UNICODE_STRING VolumeGuid;
    DS_VOLUME_PROPERTIES VolumeProperties;
    DS_FILESYSTEM_PROPERTIES FileSystemProperties;
} DS_INSTANCE_CONTEXT, *PDS_INSTANCE_CONTEXT;

typedef struct _DS_FILE_CONTEXT {
    void *nothing;
} DS_FILE_CONTEXT, *PDS_FILE_CONTEXT;

typedef struct _DS_STREAM_CONTEXT {
    void *nothing;
} DS_STREAM_CONTEXT, *PDS_STREAM_CONTEXT;

NTSTATUS DsInitInstanceContext(_In_ PCFLT_RELATED_OBJECTS FltObjects, _Inout_ PDS_INSTANCE_CONTEXT Context);
VOID DsFreeInstanceContext(_In_ PDS_INSTANCE_CONTEXT Context);

NTSTATUS DsInitFileContext(_Inout_ PDS_FILE_CONTEXT FileContext);
VOID DsFreeFileContext(_In_ PDS_FILE_CONTEXT FileContext);

NTSTATUS DsInitStreamContext(_Inout_ PDS_STREAM_CONTEXT StreamContext);
VOID DsFreeStreamContext(_In_ PDS_STREAM_CONTEXT Context);
