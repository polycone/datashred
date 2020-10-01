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

typedef struct _DS_STREAM_CONTEXT {
    UNICODE_STRING FileName;
} DS_STREAM_CONTEXT, *PDS_STREAM_CONTEXT;

NTSTATUS DsInitStreamContext(_In_ PFLT_FILE_NAME_INFORMATION FileNameInfo, _Inout_ PDS_STREAM_CONTEXT Context);
VOID DsFreeStreamContext(PDS_STREAM_CONTEXT Context);
