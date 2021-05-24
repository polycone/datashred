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

#ifdef _WIN64
#define PUSH_LOCK_ALIGN __declspec(align(8))
#else
#define PUSH_LOCK_ALIGN __declspec(align(4))
#endif // _WIN64

typedef struct _DS_FILE_STATE {
    LONG HandleCount;
    union {
        struct {
            UINT8 Locked : 1;
        };
        ULONG Flags;
    };
} DS_FILE_STATE, *PDS_FILE_STATE;

typedef struct _DS_FILE_CONTEXT {
    PUSH_LOCK_ALIGN EX_PUSH_LOCK Lock;
    DS_FILE_STATE State;
} DS_FILE_CONTEXT, *PDS_FILE_CONTEXT;

typedef struct _DS_STREAM_CONTEXT {
    PDS_FILE_CONTEXT FileContext;
    struct {
        UINT8 Main : 1;
    };
    DS_FILE_STATE State;

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
    _In_ PDS_FILE_CONTEXT FileContext,
    _In_ PFLT_FILE_NAME_INFORMATION FileNameInformation,
    _Outptr_ PDS_STREAM_CONTEXT *Context
);

VOID DsCleanupStreamContext(_Inout_ PDS_STREAM_CONTEXT Context);
