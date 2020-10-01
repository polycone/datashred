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
#include "context/stream.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, DsPreCreateCallback)
#pragma alloc_text(PAGE, DsPostCreateCallback)
#endif

static DECLARE_CONST_UNICODE_STRING(DataStreamTypeName, L"$DATA");

static BOOLEAN IsDataStream(PFLT_FILE_NAME_INFORMATION FileNameInfo);

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

    PFLT_FILE_NAME_INFORMATION fileNameInfo = NULL;
    DSR_ASSERT(FltGetFileNameInformation(Data, FLT_FILE_NAME_OPENED | FLT_FILE_NAME_QUERY_ALWAYS_ALLOW_CACHE_LOOKUP, &fileNameInfo));

    ULONG disposition = Data->Iopb->Parameters.Create.Options >> 24;
    if (disposition == FILE_SUPERSEDE ||
        disposition == FILE_OVERWRITE ||
        disposition == FILE_OVERWRITE_IF
    ) {
        // TODO: Try to open a file (with forced access check).
        // TODO: If everything was fine process the file.
    }

    *CompletionContext = fileNameInfo;
    DSR_CLEANUP_START();
    if (fileNameInfo != NULL) {
        FltReleaseFileNameInformation(fileNameInfo);
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
    DSR_INIT(PASSIVE_LEVEL);

    /*
        Ignore processing in case of:
          - Draining state
          - Invalid I/O status
          - Reparse point
          - File is a directory
          - Non-data stream type
    */

    PFLT_FILE_NAME_INFORMATION fileNameInfo = (PFLT_FILE_NAME_INFORMATION)CompletionContext;
    PDS_STREAM_CONTEXT streamContext = NULL_CONTEXT;

    if (FlagOn(Flags, FLTFL_POST_OPERATION_DRAINING)) {
        DSR_CLEANUP();
    }

    if (!NT_SUCCESS(Data->IoStatus.Status) || Data->IoStatus.Status == STATUS_REPARSE) {
        DSR_CLEANUP();
    }

    FILE_STANDARD_INFORMATION fileStandardInfo;
    DSR_ASSERT(FltQueryInformationFile(FltObjects->Instance, FltObjects->FileObject, &fileStandardInfo, sizeof(FILE_STANDARD_INFORMATION), FileStandardInformation, NULL));
    if (fileStandardInfo.Directory) {
        DSR_CLEANUP();
    }

    DSR_ASSERT(FltParseFileNameInformation(fileNameInfo));
    if (!IsDataStream(fileNameInfo)) {
        DSR_CLEANUP();
    }

    BOOLEAN streamContextSupported = FltSupportsStreamContexts(FltObjects->FileObject);
    if (!streamContextSupported) {
        // TODO: Set stream context not supported status.
        DSR_CLEANUP();
    }

    DSR_ASSERT(
        FltGetStreamContext(FltObjects->Instance, FltObjects->FileObject, &streamContext),
        DSR_SUPPRESS(STATUS_NOT_FOUND)
    );
    if (streamContext == NULL_CONTEXT) {
        DSR_ASSERT(FltAllocateContext(FltObjects->Filter, FLT_STREAM_CONTEXT, sizeof(DS_STREAM_CONTEXT), PagedPool, &streamContext));
        DSR_ASSERT(DsInitStreamContext(fileNameInfo, streamContext));
        PDS_STREAM_CONTEXT oldStreamContext;
        DSR_ASSERT(
            FltSetStreamContext(FltObjects->Instance, FltObjects->FileObject, FLT_SET_CONTEXT_KEEP_IF_EXISTS, streamContext, &oldStreamContext),
            DSR_SUPPRESS(STATUS_FLT_CONTEXT_ALREADY_DEFINED)
        );
        if (oldStreamContext != NULL_CONTEXT) {
            DsLogTrace("Stream context concurrent initialization detected.");
            FltReleaseContext(streamContext);
            streamContext = oldStreamContext;
        }
    }

    // TODO: Increase file handles count

    DsLogTrace("File opened: %wZ", &fileNameInfo->Name);

    DSR_CLEANUP_START();
    // TODO: Use FltSendMessage to signal user-mode agent that a file cannot be processed.
    // TODO: Depening on an agent response try to call FltCancelFileOpen or just finish processing.
    DSR_CLEANUP_END();

    if (streamContext != NULL_CONTEXT) {
        FltReleaseContext(streamContext);
    }
    FltReleaseFileNameInformation(fileNameInfo);
    return FLT_POSTOP_FINISHED_PROCESSING;
}

static BOOLEAN IsDataStream(PFLT_FILE_NAME_INFORMATION FileNameInfo) {
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
