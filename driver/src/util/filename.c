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

#include "filename.h"

static DECLARE_CONST_UNICODE_STRING(DataStreamTypeName, L"$DATA");

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, DsIsDataStream)
#pragma alloc_text(PAGE, DsIsDefaultStream)
#endif

BOOLEAN DsIsDataStream(PFLT_FILE_NAME_INFORMATION FileNameInfo) {
    PUNICODE_STRING streamName = &FileNameInfo->Stream;
    if (streamName->Length == 0) {
        return TRUE;
    }
    int typeNameIndex = -1;
    for (int i = (streamName->Length / sizeof(WCHAR)) - 1; i >= 0; --i) {
        if (streamName->Buffer[i] == L':') {
            typeNameIndex = i + 1;
            break;
        }
    }
    if (typeNameIndex == 1) {
        return TRUE;
    }
    USHORT typeNameLength = streamName->Length - (USHORT)typeNameIndex * sizeof(WCHAR);
    UNICODE_STRING typeName = { typeNameLength, typeNameLength, &streamName->Buffer[typeNameIndex] };
    return RtlCompareUnicodeString(&typeName, &DataStreamTypeName, TRUE) == 0;
}

BOOLEAN DsIsDefaultStream(PFLT_FILE_NAME_INFORMATION FileNameInfo) {
    if (FileNameInfo->Stream.Length == 0) {
        return TRUE;
    }
    if (FileNameInfo->Stream.Length >= 2 &&
        FileNameInfo->Stream.Buffer[0] == L':' &&
        FileNameInfo->Stream.Buffer[1] == L':'
    ) {
        return TRUE;
    }
    return FALSE;
}
