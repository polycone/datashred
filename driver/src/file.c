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
#include "file.h"

static DECLARE_CONST_UNICODE_STRING(DataStreamTypeName, L"$DATA");

#define DeclareObjectAttributes(var, name, attributes, root) \
    OBJECT_ATTRIBUTES var;                          \
    var.Length = sizeof(OBJECT_ATTRIBUTES);         \
    var.ObjectName = name;                          \
    var.Attributes = attributes;                    \
    var.RootDirectory = root;                       \
    var.SecurityDescriptor = NULL;                  \
    var.SecurityQualityOfService = NULL;

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, DsIsDataStream)
#pragma alloc_text(PAGE, DsIsAlternateStream)
#pragma alloc_text(PAGE, DsCreateFile)
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

BOOLEAN DsIsAlternateStream(_In_ PUNICODE_STRING StreamName) {
    if (StreamName->Length == 0)
        return FALSE;
    BOOLEAN doubleColon = StreamName->Length >= 2 && StreamName->Buffer[0] == L':' && StreamName->Buffer[1] == L':';
    return !doubleColon;
}

NTSTATUS DsCreateFile(
    PCFLT_RELATED_OBJECTS FltObjects,
    PUNICODE_STRING Name,
    ACCESS_MASK DesiredAccess,
    ULONG ShareAccess,
    DS_FILE_CREATE_OPTIONS Options,
    _Out_ PDS_FILE_DESCRIPTOR Descriptor
) {
    NT_ASSERT(KeGetCurrentIrql() <= PASSIVE_LEVEL);
    HANDLE handle = NULL;
    PFILE_OBJECT fileObject = NULL;
    IO_STATUS_BLOCK ioStatus;
    ULONG flags = Options & DS_FILE_IO_FLAGS_MASK;
    DeclareObjectAttributes(attributes, Name, OBJ_KERNEL_HANDLE, NULL);
    if (FlagOn(Options, DS_FILE_FORCE_ACCESS_CHECK))
        attributes.Attributes |= OBJ_FORCE_ACCESS_CHECK;
    NTSTATUS status = FltCreateFileEx2(
        FltObjects->Filter,
        FltObjects->Instance,
        &handle,
        &fileObject,
        DesiredAccess,
        &attributes,
        &ioStatus,
        /* AllocationSize = */ NULL,
        /* FileAttributes = */ 0,
        ShareAccess,
        FILE_OPEN,
        FILE_COMPLETE_IF_OPLOCKED,
        /* EaBuffer = */ NULL,
        /* EaLength = */ 0,
        flags,
        /* DriverContext = */ NULL
    );
    if (NT_SUCCESS(status)) {
        Descriptor->Handle = handle;
        Descriptor->FileObject = fileObject;
    }
    return status;
}
