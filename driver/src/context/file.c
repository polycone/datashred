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

static VOID DsExtractFileName(_In_ PFLT_FILE_NAME_INFORMATION FileNameInfo, _Out_ PUNICODE_STRING FileName);

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, DsInitFileContext)
#pragma alloc_text(PAGE, DsFreeFileContext)
#pragma alloc_text(PAGE, DsExtractFileName)
#endif

NTSTATUS DsInitFileContext(
    _Inout_ PDS_FILE_CONTEXT FileContext,
    _In_ PFLT_FILE_NAME_INFORMATION FileNameInfo,
    _In_ PDS_INSTANCE_CONTEXT InstanceContext
) {
    DSR_INIT(APC_LEVEL);
    UNICODE_STRING fileName;
    DsExtractFileName(FileNameInfo, &fileName);
    DSR_ASSERT(DsInitMonitorContext(&FileContext->MonitorContext, &fileName, InstanceContext));
    DSR_CLEANUP_EMPTY();
    return DSR_STATUS;
}

VOID DsFreeFileContext(_In_ PDS_FILE_CONTEXT FileContext) {
    DsFreeMonitorContext(&FileContext->MonitorContext);
}

static VOID DsExtractFileName(_In_ PFLT_FILE_NAME_INFORMATION FileNameInfo, _Out_ PUNICODE_STRING FileName) {
    FileName->Buffer = FileNameInfo->Name.Buffer;
    FileName->Length = FileNameInfo->Name.Length - FileNameInfo->Stream.Length;
    FileName->MaximumLength = FileName->Length;
}
