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

NTSTATUS DsAllocateUnicodeString(_Out_ PUNICODE_STRING String, USHORT Length);
VOID DsFreeUnicodeString(_Inout_ PUNICODE_STRING String);
NTSTATUS DsCopyUnicodeString(_Inout_ PUNICODE_STRING Destination, _In_ PUNICODE_STRING Source);

#define EmptyUnicodeString                          { 0, 0, NULL }
#define DsInitUnicodeString(string)                 DsAllocateUnicodeString(string, 0)
