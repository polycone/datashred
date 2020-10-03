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

#include "stream.h"
#include "util/string.h"
#include "util/filename.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, DsInitStreamContext)
#pragma alloc_text(PAGE, DsFreeStreamContext)
#endif

NTSTATUS DsInitStreamContext(
    _In_ PFLT_FILE_NAME_INFORMATION FileNameInfo,
    _In_ PDS_INSTANCE_CONTEXT InstanceContext,
    _In_opt_ PDS_FILE_CONTEXT FileContext,
    _Inout_ PDS_STREAM_CONTEXT StreamContext
) {
    DSR_INIT(APC_LEVEL);
    DSR_ASSERT(DsCreateUnicodeString(&StreamContext->FileName, FileNameInfo->Name.Length));
    RtlCopyUnicodeString(&StreamContext->FileName, &FileNameInfo->Name);
    StreamContext->HandleCount = 0;
    StreamContext->DefaultStream = DsIsDefaultStream(FileNameInfo);
    StreamContext->InstanceContext = InstanceContext;
    StreamContext->FileContext = FileContext;
    FltReferenceContext(InstanceContext);
    if (FileContext != NULL) {
        FltReferenceContext(FileContext);
    }
    DSR_CLEANUP_EMPTY();
    return DSR_STATUS;
}

VOID DsFreeStreamContext(PDS_STREAM_CONTEXT Context) {
    FltReleaseContext(Context->InstanceContext);
    if (Context->FileContext != NULL) {
        FltReleaseContext(Context->FileContext);
    }
    DsFreeUnicodeString(&Context->FileName);
}
