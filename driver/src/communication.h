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

typedef struct _DS_COMMUNICATION {
	UNICODE_STRING Name;
	PFLT_PORT Port;
	PFLT_CONNECT_NOTIFY OnConnect;
	PFLT_DISCONNECT_NOTIFY OnDisconnect;
	PFLT_MESSAGE_NOTIFY OnMessage;
	LONG MaxConnections;
	BOOLEAN Initialized;
} DS_COMMUNICATION, *PDS_COMMUNICATION;

NTSTATUS DsCreateCommunication(_Inout_ PDS_COMMUNICATION Communication);
VOID DsCloseCommunication(_In_ PDS_COMMUNICATION Communication);

#define SetupCommunication(Communication, Name, OnConnect, OnDisconnect, OnMessage) { \
	(Communication)->Name = Name \
	(Communication)->Port = NULL \
	(Communication)->OnConnect = OnConnect \
	(Communication)->OnDisconnect = OnDisconnect \
	(Communication)->OnMessage = OnMessage \
	(Communication)->MaxConnections = 1 \
	(Communication)->Initialized = FALSE \
}
