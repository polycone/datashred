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
#include "driver.h"
#include "volume.h"

typedef struct _DS_INSTANCE_CONTEXT {
    UNICODE_STRING VolumeGuid;
    DS_VOLUME_PROPERTIES VolumeProperties;
    DS_FILESYSTEM_PROPERTIES FileSystemProperties;
} DS_INSTANCE_CONTEXT, *PDS_INSTANCE_CONTEXT;

typedef struct _DS_FILE_CONTEXT {
    POINTER_ALIGNMENT EX_PUSH_LOCK Lock;
    LONG HandleCount;
    union {
        struct {
            BOOLEAN Locked : 1;
            BOOLEAN DeleteOnClose : 1;
        };
        ULONG Flags;
    };
} DS_FILE_CONTEXT, *PDS_FILE_CONTEXT;

typedef struct _DS_STREAM_CONTEXT {
    PDS_INSTANCE_CONTEXT InstanceContext;
    PDS_FILE_CONTEXT FileContext;
    LONG HandleCount;
    union {
        struct {
            BOOLEAN Locked : 1;
            BOOLEAN DeleteOnClose : 1;
            BOOLEAN AlternateStream : 1;
        };
        ULONG Flags;
    };

#ifdef DBG
    UNICODE_STRING Name;
#endif
} DS_STREAM_CONTEXT, *PDS_STREAM_CONTEXT;

NTSTATUS DsCreateInstanceContext(_In_ PCFLT_RELATED_OBJECTS FltObjects, _Out_ PDS_INSTANCE_CONTEXT *Context);
VOID DsCleanupInstanceContext(_Inout_ PDS_INSTANCE_CONTEXT Context);

NTSTATUS DsCreateFileContext(_In_ PCFLT_RELATED_OBJECTS FltObjects, _Outptr_ PDS_FILE_CONTEXT *Context);
VOID DsCleanupFileContext(_Inout_ PDS_FILE_CONTEXT Context);

NTSTATUS DsCreateStreamContext(
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ PDS_INSTANCE_CONTEXT InstanceContext,
    _In_ PDS_FILE_CONTEXT FileContext,
    _In_ PFLT_FILE_NAME_INFORMATION FileNameInformation,
    _Outptr_ PDS_STREAM_CONTEXT *Context
);

VOID DsCleanupStreamContext(_Inout_ PDS_STREAM_CONTEXT Context);

NTSTATUS DsStreamAddHandle(_In_ PDS_STREAM_CONTEXT StreamContext, _In_ PFLT_CALLBACK_DATA Data);
VOID DsStreamRemoveHandle(_In_ PDS_STREAM_CONTEXT StreamContext);
