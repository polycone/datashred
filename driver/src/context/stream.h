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
#include "file.h"
#include "instance.h"

typedef struct _DS_STREAM_CONTEXT {
    PDS_INSTANCE_CONTEXT InstanceContext;
    PDS_FILE_CONTEXT FileContext;
    UNICODE_STRING FileName;
    volatile LONG HandleCount;
    struct {
        BOOLEAN DefaultStream : 1;
        BOOLEAN DeleteOnClose : 1;
    };
} DS_STREAM_CONTEXT, *PDS_STREAM_CONTEXT;

NTSTATUS DsInitStreamContext(
    _In_ PFLT_FILE_NAME_INFORMATION FileNameInfo,
    _In_ PDS_INSTANCE_CONTEXT instanceContext,
    _In_opt_ PDS_FILE_CONTEXT fileContext,
    _Inout_ PDS_STREAM_CONTEXT StreamContext
);
VOID DsFreeStreamContext(_In_ PDS_STREAM_CONTEXT Context);
VOID DsMarkStreamAsDeleteOnClose(_In_ PDS_STREAM_CONTEXT Context);
