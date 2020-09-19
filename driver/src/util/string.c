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

#include "string.h"
#include "memory.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, DsAllocUnicodeString)
#pragma alloc_text(PAGE, DsEnsureUnicodeStringLength)
#pragma alloc_text(PAGE, DsFreeUnicodeString)
#endif

NTSTATUS DsAllocUnicodeString(_In_ USHORT Length, _Inout_ PUNICODE_STRING *String) {
    PUNICODE_STRING string = NULL;
    NTSTATUS status = DsMemAlloc(sizeof(UNICODE_STRING) + Length, &string);
    if (!NT_SUCCESS(status)) {
        return status;
    }
    string->Buffer = (PWCH)((ULONG_PTR)string + sizeof(UNICODE_STRING));
    string->Length = 0;
    string->MaximumLength = Length;
    *String = string;
    return STATUS_SUCCESS;
}

NTSTATUS DsEnsureUnicodeStringLength(_Inout_ PUNICODE_STRING String, _In_ USHORT Length) {
    if (String->MaximumLength >= Length) {
        String->Length = 0;
        return STATUS_SUCCESS;
    }
    PVOID buffer = NULL;
    NTSTATUS status = DsMemAlloc(Length, &buffer);
    if (!NT_SUCCESS(status)) {
        return status;
    }
    if (String->Buffer != NULL) {
        DsMemFree(String->Buffer);
    }
    String->Buffer = (PWCH)buffer;
    String->Length = 0;
    String->MaximumLength = Length;
    return STATUS_SUCCESS;
}

VOID DsFreeUnicodeString(_In_ PUNICODE_STRING String) {
    DsMemFree(String->Buffer);
    String->Buffer = NULL;
    String->MaximumLength = 0;
    String->Length = 0;
}
