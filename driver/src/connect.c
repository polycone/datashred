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

#include "connect.h"
#include "memory.h"

#define DSC_MAX_CONNECTIONS 1

static NTSTATUS FLTAPI ConnectNotifyCallback(
    _In_ PFLT_PORT ClientPort,
    _In_opt_ PVOID ServerPortCookie,
    _In_reads_bytes_opt_(SizeOfContext) PVOID ConnectionContext,
    _In_ ULONG SizeOfContext,
    _Outptr_result_maybenull_ PVOID *ConnectionPortCookie
);

static VOID FLTAPI DisconnectNotifyCallback(_In_opt_ PVOID ConnectionCookie);

static NTSTATUS FLTAPI MessageNotifyCallback(
    _In_opt_ PVOID PortCookie,
    _In_reads_bytes_opt_(InputBufferLength) PVOID InputBuffer,
    _In_ ULONG InputBufferLength,
    _Out_writes_bytes_to_opt_(OutputBufferLength, *ReturnOutputBufferLength) PVOID OutputBuffer,
    _In_ ULONG OutputBufferLength,
    _Out_ PULONG ReturnOutputBufferLength
);

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, DsCreateConnect)
#pragma alloc_text(PAGE, DsFreeConnect)
#endif

NTSTATUS DsCreateConnect(
    _In_ PFLT_FILTER Filter,
    _In_ PCWSTR Name,
    _In_ DSC_MESSAGE_NOTIFY MessageNotifyCallback,
    _In_ PVOID Context,
    _Out_ PDS_CONNECT *Connect
) {
    PAGED_CODE();

    NTSTATUS status = STATUS_SUCCESS;
    PDS_CONNECT pConnect = NULL;
    PSECURITY_DESCRIPTOR pSecurityDescriptor = NULL;

    status = DsMemAllocType(DS_CONNECT, &pConnect);
    if (!NT_SUCCESS(status)) goto cleanup;
    RtlZeroMemory(pConnect, sizeof(DS_CONNECT));
    pConnect->MessageNotifyCallback = MessageNotifyCallback;
    pConnect->Context = Context;

    status = FltBuildDefaultSecurityDescriptor(&pSecurityDescriptor, FLT_PORT_ALL_ACCESS);
    if (!NT_SUCCESS(status)) goto cleanup;

    OBJECT_ATTRIBUTES attributes;
    UNICODE_STRING name;
    RtlInitUnicodeString(&name, Name);
    InitializeObjectAttributes(
        &attributes,
        &name,
        OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE,
        NULL,
        pSecurityDescriptor
    );

    status = FltCreateCommunicationPort(
        Filter,
        &pConnect->ServerPort,
        &attributes,
        pConnect,
        ConnectNotifyCallback,
        DisconnectNotifyCallback,
        MessageNotifyCallback,
        DSC_MAX_CONNECTIONS
    );
    if (!NT_SUCCESS(status)) goto cleanup;

    FltFreeSecurityDescriptor(pSecurityDescriptor);
    *Connect = pConnect;
    return status;

cleanup:
    if (pConnect != NULL) {
        DsMemFree(pConnect);
    }
    if (pSecurityDescriptor != NULL) {
        FltFreeSecurityDescriptor(pSecurityDescriptor);
    }
    return status;
}

VOID DsFreeConnect(_In_ PDS_CONNECT Connect) {
    if (Connect->ClientPort != NULL) {
        FltCloseClientPort(Connect->Filter, &Connect->ClientPort);
    }
    FltCloseCommunicationPort(Connect->ServerPort);
    DsMemFree(Connect);
}

static NTSTATUS FLTAPI ConnectNotifyCallback(
    _In_ PFLT_PORT ClientPort,
    _In_opt_ PVOID ServerPortCookie,
    _In_reads_bytes_opt_(SizeOfContext) PVOID ConnectionContext,
    _In_ ULONG SizeOfContext,
    _Outptr_result_maybenull_ PVOID *ConnectionPortCookie
) {
    UNREFERENCED_PARAMETER(ClientPort);
    UNREFERENCED_PARAMETER(ConnectionContext);
    UNREFERENCED_PARAMETER(SizeOfContext);
    *ConnectionPortCookie = ServerPortCookie;
    return STATUS_SUCCESS;
}

static VOID FLTAPI DisconnectNotifyCallback(
    _In_opt_ PVOID ConnectionCookie
) {
    UNREFERENCED_PARAMETER(ConnectionCookie);
}

static NTSTATUS FLTAPI MessageNotifyCallback(
    _In_opt_ PVOID PortCookie,
    _In_reads_bytes_opt_(InputBufferLength) PVOID InputBuffer,
    _In_ ULONG InputBufferLength,
    _Out_writes_bytes_to_opt_(OutputBufferLength, *ReturnOutputBufferLength) PVOID OutputBuffer,
    _In_ ULONG OutputBufferLength,
    _Out_ PULONG ReturnOutputBufferLength
) {
    PDS_CONNECT pConnect = (PDS_CONNECT)PortCookie;
    return pConnect->MessageNotifyCallback(
        pConnect->Context,
        InputBuffer,
        InputBufferLength,
        OutputBuffer,
        OutputBufferLength,
        ReturnOutputBufferLength
    );
}
