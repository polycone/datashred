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

#include <driver.h>

static DECLARE_CONST_UNICODE_STRING(DataStreamTypeName, L"$DATA");

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, DsIsDataStream)
#pragma alloc_text(PAGE, DsIsDefaultStream)
#endif

BOOLEAN DsIsDataStream(_In_ PUNICODE_STRING StreamName) {
    if (StreamName->Length == 0)
        return TRUE;
    int typeNameIndex = -1;
    for (int i = (StreamName->Length / sizeof(WCHAR)) - 1; i >= 0; --i) {
        if (StreamName->Buffer[i] == L':') {
            typeNameIndex = i + 1;
            break;
        }
    }
    if (typeNameIndex == 1)
        return TRUE;
    USHORT typeNameLength = StreamName->Length - (USHORT)typeNameIndex * sizeof(WCHAR);
    UNICODE_STRING typeName = { typeNameLength, typeNameLength, &StreamName->Buffer[typeNameIndex] };
    return RtlCompareUnicodeString(&typeName, &DataStreamTypeName, TRUE) == 0;
}

BOOLEAN DsIsDefaultStream(_In_ PUNICODE_STRING StreamName) {
    if (StreamName->Length == 0)
        return TRUE;
    BOOLEAN doubleDot = StreamName->Length >= 2 && StreamName->Buffer[0] == L':' && StreamName->Buffer[1] == L':';
    return doubleDot;
}
