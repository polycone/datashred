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

typedef NTSTATUS(FLTAPI *DSC_MESSAGE_NOTIFY) (
    _In_ PVOID Context,
    _In_reads_bytes_opt_(InputBufferLength) PVOID InputBuffer,
    _In_ ULONG InputBufferLength,
    _Out_writes_bytes_to_opt_(OutputBufferLength, *ReturnOutputBufferLength) PVOID OutputBuffer,
    _In_ ULONG OutputBufferLength,
    _Out_ PULONG ReturnOutputBufferLength
);

typedef struct _DS_CONNECT {
    PFLT_PORT ServerPort;
    PFLT_PORT ClientPort;
    DSC_MESSAGE_NOTIFY MessageNotifyCallback;
    PVOID Context;
} DS_CONNECT, *PDS_CONNECT;

NTSTATUS DsCreateConnect(
    _In_ PCWSTR Name,
    _In_ DSC_MESSAGE_NOTIFY MessageNotifyCallback,
    _In_ PVOID Context,
    _Out_ PDS_CONNECT *Connect
);

VOID DsFreeConnect(_In_ PDS_CONNECT Connect);
