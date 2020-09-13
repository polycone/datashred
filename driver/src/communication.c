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

#include "datashred.h"
#include "communication.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, DsCreateCommunication)
#pragma alloc_text(PAGE, DsCloseCommunication)
#endif

NTSTATUS DsCreateCommunication(_Inout_ PDS_COMMUNICATION Communication) {
	PAGED_CODE();
	NTSTATUS status = STATUS_SUCCESS;
	PSECURITY_DESCRIPTOR pSecurityDescriptor = NULL;
	status = FltBuildDefaultSecurityDescriptor(&pSecurityDescriptor, FLT_PORT_ALL_ACCESS);
	if (!NT_SUCCESS(status)) return status;

	OBJECT_ATTRIBUTES attributes;
	InitializeObjectAttributes(
		&attributes,
		&Communication->Name,
		OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE,
		NULL,
		pSecurityDescriptor
	);

	status = FltCreateCommunicationPort(
		State.Filter,
		&Communication->Port,
		&attributes,
		NULL,
		Communication->OnConnect,
		Communication->OnDisconnect,
		Communication->OnMessage,
		Communication->MaxConnections
	);
	Communication->Initialized = NT_SUCCESS(status);
	FltFreeSecurityDescriptor(pSecurityDescriptor);
	return status;
}

VOID DsCloseCommunication(_In_ PDS_COMMUNICATION Communication) {
	FltCloseCommunicationPort(Communication->Port);
}
