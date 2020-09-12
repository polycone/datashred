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

NTSTATUS DriverEntry(_In_ PDRIVER_OBJECT driverObject, _In_ PUNICODE_STRING pRegistryPath);
static NTSTATUS DsFilterLoad(_In_ PDRIVER_OBJECT driverObject, _In_ PUNICODE_STRING pRegistryPath);
static NTSTATUS DsFilterUnload(_In_ FLT_FILTER_UNLOAD_FLAGS flags);

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(INIT, DsFilterLoad)
#pragma alloc_text(PAGE, DsFilterUnload)
#endif

static const FLT_OPERATION_REGISTRATION callbacks[] = {
	{ IRP_MJ_OPERATION_END }
};

static const FLT_REGISTRATION filterRegistration = {
	sizeof(FLT_REGISTRATION),
	FLT_REGISTRATION_VERSION,
	FLTFL_NONE,
	NO_CONTEXT,
	callbacks,
	DsFilterUnload,
	NO_CALLBACK,
	NO_CALLBACK,
	NO_CALLBACK,
	NO_CALLBACK,
	NO_CALLBACK,
	NO_CALLBACK,
	NO_CALLBACK,
	NO_CALLBACK,
	NO_CALLBACK
};

NTSTATUS DriverEntry(_In_ PDRIVER_OBJECT pDriverObject, _In_ PUNICODE_STRING pRegistryPath) {
	return DsFilterLoad(pDriverObject, pRegistryPath);
}

static NTSTATUS DsFilterLoad(_In_ PDRIVER_OBJECT pDriverObject, _In_ PUNICODE_STRING pRegistryPath) {
	UNREFERENCED_PARAMETER(pRegistryPath);
	NTSTATUS status = FltRegisterFilter(pDriverObject, &filterRegistration, &State.Filter);
	FLT_ASSERT(NT_SUCCESS(status));
	if (NT_SUCCESS(status)) {
		status = FltStartFiltering(State.Filter);
		if (!NT_SUCCESS(status)) {
			FltUnregisterFilter(State.Filter);
		}
	}
	return status;
}

static NTSTATUS DsFilterUnload(_In_ FLT_FILTER_UNLOAD_FLAGS flags) {
	UNREFERENCED_PARAMETER(flags);
	PAGED_CODE();
	FltUnregisterFilter(State.Filter);
	return STATUS_SUCCESS;
}
