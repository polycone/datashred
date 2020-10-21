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
#include "instance.h"

#define DS_MONITOR_FLAGS    ULONG

#define DS_MONITOR_FILE_DEFAULT_STREAM         0x00000001
#define DS_MONITOR_FILE_DELETE_ON_CLOSE        0x00000002

#ifdef _WIN64
#define PUSH_LOCK_ALIGN __declspec(align(8))
#else
#define PUSH_LOCK_ALIGN __declspec(align(4))
#endif // _WIN64

typedef struct _DS_MONITOR_CONTEXT {
    PDS_INSTANCE_CONTEXT InstanceContext;
    UNICODE_STRING Name;
    volatile LONG HandleCount;
    DS_MONITOR_FLAGS Flags;
    PUSH_LOCK_ALIGN EX_PUSH_LOCK Lock;
} DS_MONITOR_CONTEXT, *PDS_MONITOR_CONTEXT;

NTSTATUS DsInitMonitorContext(
    _Inout_ PDS_MONITOR_CONTEXT MonitorContext,
    _In_ PUNICODE_STRING FileName,
    _In_ PDS_INSTANCE_CONTEXT InstanceContext
);

VOID DsFreeMonitorContext(_In_ PDS_MONITOR_CONTEXT MonitorContext);
