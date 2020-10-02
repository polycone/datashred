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

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, DsInitStreamContext)
#pragma alloc_text(PAGE, DsFreeStreamContext)
#endif

NTSTATUS DsInitStreamContext(_In_ PFLT_FILE_NAME_INFORMATION FileNameInfo, _Inout_ PDS_STREAM_CONTEXT Context) {
    DSR_INIT(APC_LEVEL);
    DSR_ASSERT(DsCreateUnicodeString(&Context->FileName, FileNameInfo->Name.Length));
    RtlCopyUnicodeString(&Context->FileName, &FileNameInfo->Name);
    Context->HandleCount = 0;
    DSR_CLEANUP_EMPTY();
    return DSR_STATUS;
}

VOID DsFreeStreamContext(PDS_STREAM_CONTEXT Context) {
    DsFreeUnicodeString(&Context->FileName);
}
