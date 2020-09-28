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

#include "create.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, DsPreCreateCallback)
#pragma alloc_text(PAGE, DsPostCreateCallback)
#endif

FLT_PREOP_CALLBACK_STATUS DsPreCreateCallback(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _Out_ PVOID *CompletionContext
) {
    UNREFERENCED_PARAMETER(FltObjects);
    DSR_INIT(PASSIVE_LEVEL);
    *CompletionContext = NULL;

    if (FlagOn(Data->Iopb->Parameters.Create.Options, FILE_DIRECTORY_FILE))
        return FLT_PREOP_SUCCESS_NO_CALLBACK;
    if (FlagOn(Data->Iopb->OperationFlags, SL_OPEN_PAGING_FILE))
        return FLT_PREOP_SUCCESS_NO_CALLBACK;
    if (FlagOn(Data->Iopb->TargetFileObject->Flags, FO_VOLUME_OPEN))
        return FLT_PREOP_SUCCESS_NO_CALLBACK;

    PFLT_FILE_NAME_INFORMATION filenameInfo = NULL;
    DSR_ASSERT(FltGetFileNameInformation(Data, FLT_FILE_NAME_OPENED, &filenameInfo));

    ULONG disposition = Data->Iopb->Parameters.Create.Options >> 24;
    if (disposition == FILE_SUPERSEDE ||
        disposition == FILE_OVERWRITE ||
        disposition == FILE_OVERWRITE_IF
    ) {
        // TODO: Try to open a file (with forced access check).
        // TODO: If everything was fine process the file.
    }

    *CompletionContext = filenameInfo;
    DSR_CLEANUP_START();
    if (filenameInfo != NULL) {
        FltReleaseFileNameInformation(filenameInfo);
    }
    Data->IoStatus.Status = DSR_STATUS;
    DSR_CLEANUP_END();

    return DSR_SUCCESS ? FLT_PREOP_SYNCHRONIZE : FLT_PREOP_COMPLETE;
}

FLT_POSTOP_CALLBACK_STATUS DsPostCreateCallback(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ PVOID CompletionContext,
    _In_ FLT_POST_OPERATION_FLAGS Flags
) {
    UNREFERENCED_PARAMETER(Flags);
    DSR_INIT(PASSIVE_LEVEL);

    if (!NT_SUCCESS(Data->IoStatus.Status)) {
        return FLT_POSTOP_FINISHED_PROCESSING;
    }

    PFLT_FILE_NAME_INFORMATION filenameInfo = (PFLT_FILE_NAME_INFORMATION)CompletionContext;
    FILE_STANDARD_INFORMATION fileStandardInfo;
    DSR_ASSERT(FltQueryInformationFile(FltObjects->Instance, FltObjects->FileObject, &fileStandardInfo, sizeof(FILE_STANDARD_INFORMATION), FileStandardInformation, NULL));
    if (fileStandardInfo.Directory) {
        return FLT_POSTOP_FINISHED_PROCESSING;
    }

    DsLogInfo("Flie opened: %wZ", &filenameInfo->Name);

    DSR_CLEANUP_EMPTY();
    FltReleaseFileNameInformation(filenameInfo);
    return FLT_POSTOP_FINISHED_PROCESSING;
}
