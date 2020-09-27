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
#pragma alloc_text(PAGE, DsCreateUnicodeString)
#pragma alloc_text(PAGE, DsEnsureUnicodeStringLength)
#pragma alloc_text(PAGE, DsFreeUnicodeString)
#endif

NTSTATUS DsCreateUnicodeString(_Inout_ PUNICODE_STRING String, _In_ USHORT Length) {
    DSR_INIT(APC_LEVEL);
    PVOID buffer = NULL;
    if (Length > 0) {
        DSR_ASSERT(DsMemAlloc(Length, &buffer));
    }
    String->Buffer = (PWCH)buffer;
    String->Length = 0;
    String->MaximumLength = Length;
    DSR_CLEANUP_EMPTY();
    return DSR_STATUS;
}

NTSTATUS DsEnsureUnicodeStringLength(_Inout_ PUNICODE_STRING String, _In_ USHORT Length) {
    DSR_INIT(APC_LEVEL);
    if (String->MaximumLength >= Length) {
        String->Length = 0;
        return DSR_STATUS;
    }
    PVOID buffer = NULL;
    DSR_ASSERT(DsMemAlloc(Length, &buffer));
    if (String->Buffer != NULL) {
        DsMemFree(String->Buffer);
    }
    String->Buffer = (PWCH)buffer;
    String->Length = 0;
    String->MaximumLength = Length;
    DSR_CLEANUP_EMPTY();
    return DSR_STATUS;
}

VOID DsFreeUnicodeString(_In_ PUNICODE_STRING String) {
    if (String->Buffer != NULL) {
        DsMemFree(String->Buffer);
    }
    String->Buffer = NULL;
    String->MaximumLength = 0;
    String->Length = 0;
}
