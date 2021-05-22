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

#include "driver.h"
#include "dsr.h"
#include "memory.h"
#include "string.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, DsAllocateUnicodeString)
#pragma alloc_text(PAGE, DsFreeUnicodeString)
#pragma alloc_text(PAGE, DsCopyUnicodeString)
#endif

NTSTATUS DsAllocateUnicodeString(_Out_ PUNICODE_STRING String, USHORT Length) {
    DSR_ENTER(APC_LEVEL);
    PVOID buffer = NULL;
    if (Length > 0) {
        DSR_ASSERT(DsMemAlloc(Length, &buffer));
    }
    String->Buffer = (PWCH)buffer;
    String->Length = 0;
    String->MaximumLength = Length;
    DSR_ERROR_HANDLER({});
    return DSR_STATUS;
}

VOID DsFreeUnicodeString(_Inout_ PUNICODE_STRING String) {
    if (String->Buffer != NULL) {
        DsMemFree(String->Buffer);
    }
    String->Buffer = NULL;
    String->MaximumLength = 0;
    String->Length = 0;
}

NTSTATUS DsCopyUnicodeString(_Inout_ PUNICODE_STRING Destination, _In_ PUNICODE_STRING Source) {
    DSR_ENTER(APC_LEVEL);
    if (Destination->MaximumLength < Source->Length) {
        DsFreeUnicodeString(Destination);
        DSR_ASSERT(DsAllocateUnicodeString(Destination, Source->Length));
    }
    RtlCopyMemory(Destination->Buffer, Source->Buffer, Source->Length);
    Destination->Length = Source->Length;
    DSR_ERROR_HANDLER({});
    return DSR_STATUS;
}
