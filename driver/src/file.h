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
#include <ntdef.h>

BOOLEAN DsIsDataStream(_In_ PUNICODE_STRING StreamName);
BOOLEAN DsIsAlternateStream(_In_ PUNICODE_STRING StreamName);

/* DsCreateFile create options */

#define DS_FILE_FORCE_ACCESS_CHECK                  0x0001
#define DS_FILE_IGNORE_SHARE_ACCESS_CHECK           0x0800
#define DS_FILE_IO_FLAGS_MASK                       0xFFFF
typedef ULONG DS_FILE_CREATE_OPTIONS;

typedef struct _DS_FILE_DESCRIPTOR {
    HANDLE Handle;
    PFILE_OBJECT FileObject;
} DS_FILE_DESCRIPTOR, *PDS_FILE_DESCRIPTOR;

#define EMPTY_FILE_DESCRIPTOR                       { .Handle = 0, .FileObject = NULL }

NTSTATUS DsCreateFile(
    PCFLT_RELATED_OBJECTS FltObjects,
    PUNICODE_STRING Name,
    ACCESS_MASK DesiredAccess,
    ULONG ShareAccess,
    DS_FILE_CREATE_OPTIONS Options,
    _Out_ PDS_FILE_DESCRIPTOR Descriptor
);

__forceinline VOID DsCloseFile(_In_ PDS_FILE_DESCRIPTOR Descriptor) {
    FltCloseSafe(Descriptor->Handle);
    ObDereferenceObjectSafe(Descriptor->FileObject);
}
