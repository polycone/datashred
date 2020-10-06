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

#include "file.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, DsInitFileContext)
#pragma alloc_text(PAGE, DsFreeFileContext)
#endif

NTSTATUS DsInitFileContext(_In_ PFLT_FILE_NAME_INFORMATION FileNameInfo, _Inout_ PDS_FILE_CONTEXT Context) {
    DSR_INIT(APC_LEVEL);
    USHORT fileNameLength = FileNameInfo->Name.Length - FileNameInfo->Stream.Length;
    DSR_ASSERT(DsCreateUnicodeString(&Context->Data.FileName, fileNameLength));
    RtlCopyUnicodeString(&Context->Data.FileName, &FileNameInfo->Name);
    Context->Data.HandleCount = 0;
    Context->Data.Flags = 0;
    ASSERT(IS_ALIGNED(&Context->Data.Lock, sizeof(void *)));
    FltInitializePushLock(&Context->Data.Lock);
    DSR_CLEANUP_EMPTY();
    return DSR_STATUS;
}

VOID DsFreeFileContext(_In_ PDS_FILE_CONTEXT Context) {
    DsFreeUnicodeString(&Context->Data.FileName);
    FltDeletePushLock(&Context->Data.Lock);
}
