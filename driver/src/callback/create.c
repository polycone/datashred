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

#include <callback.h>
#include <context.h>
#include <dsr.h>

typedef struct _DS_INITIALIZATION_CONTEXT {
    PCFLT_RELATED_OBJECTS FltObjects;
    PFLT_FILE_NAME_INFORMATION FileNameInfo;
    PDS_INSTANCE_CONTEXT InstanceContext;
    PDS_FILE_CONTEXT FileContext;
    PDS_STREAM_CONTEXT StreamContext;
} DS_INITIALIZTION_CONTEXT, *PDS_INITIALIZTION_CONTEXT;

static NTSTATUS SetupFileContext(_Inout_ PDS_INITIALIZTION_CONTEXT Context);
static NTSTATUS SetupStreamContext(_Inout_ PDS_INITIALIZTION_CONTEXT Context);

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, DsPreCreateCallback)
#pragma alloc_text(PAGE, DsPostCreateCallback)
#pragma alloc_text(PAGE, SetupFileContext)
#pragma alloc_text(PAGE, SetupStreamContext)
#endif

FLT_PREOP_CALLBACK_STATUS DsPreCreateCallback(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _Out_ PVOID *CompletionContext
) {
    DSR_ENTER(PASSIVE_LEVEL);
    *CompletionContext = NULL;
    PFLT_FILE_NAME_INFORMATION fileNameInfo = NULL;

    if (FlagOn(Data->Iopb->Parameters.Create.Options, FILE_DIRECTORY_FILE))
        DSR_RETURN(FLT_PREOP_SUCCESS_NO_CALLBACK);
    if (FlagOn(Data->Iopb->OperationFlags, SL_OPEN_PAGING_FILE))
        DSR_RETURN(FLT_PREOP_SUCCESS_NO_CALLBACK);
    if (FlagOn(Data->Iopb->TargetFileObject->Flags, FO_VOLUME_OPEN))
        DSR_RETURN(FLT_PREOP_SUCCESS_NO_CALLBACK);

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

    DSR_STATUS = DSR_SUCCESS ? FLT_PREOP_SYNCHRONIZE : FLT_PREOP_COMPLETE;
    DSR_ERROR_HANDLER({
        if (fileNameInfo != NULL)
            FltReleaseFileNameInformation(fileNameInfo);
        Data->IoStatus.Status = DSR_STATUS;
    });

    return DSR_STATUS;
}

FLT_POSTOP_CALLBACK_STATUS DsPostCreateCallback(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ PVOID CompletionContext,
    _In_ FLT_POST_OPERATION_FLAGS Flags
) {
    DSR_ENTER(PASSIVE_LEVEL);

    /*
        Ignore processing in case of:
          - Draining state
          - Invalid I/O status
          - Reparse point
          - File is a directory
          - Non-data stream type
    */

    DSR_DECLARE(context, DS_INITIALIZTION_CONTEXT);
    context.FltObjects = FltObjects;
    context.FileNameInfo = (PFLT_FILE_NAME_INFORMATION)CompletionContext;

    if (FlagOn(Flags, FLTFL_POST_OPERATION_DRAINING))
        DSR_LEAVE();
    if (!NT_SUCCESS(Data->IoStatus.Status))
        DSR_LEAVE();
    if (Data->IoStatus.Status == STATUS_REPARSE)
        DSR_LEAVE();

    FILE_STANDARD_INFORMATION fileStandardInfo;
    DSR_ASSERT(DsQueryStandardInformationFile(FltObjects, &fileStandardInfo));
    if (fileStandardInfo.Directory)
        DSR_LEAVE();

    DSR_ASSERT(FltParseFileNameInformation(context.FileNameInfo));
    if (!DsIsDataStream(&context.FileNameInfo->Stream))
        DSR_LEAVE();

    DSR_ASSERT(FltGetInstanceContext(FltObjects->Instance, &context.InstanceContext));
    DSR_ASSERT(SetupFileContext(&context));
    DSR_ASSERT(SetupStreamContext(&context));

    DSR_ERROR_HANDLER({
        // TODO: Check for STATUS_STREAM_CONTEXT_NOT_SUPPORTED
        // TODO: Use FltSendMessage to signal user-mode agent that a file cannot be processed.
        // TODO: Depening on an agent response try to call FltCancelFileOpen or just finish processing.
    });

    FltReleaseContextSafe(context.StreamContext);
    FltReleaseContextSafe(context.FileContext);
    FltReleaseContextSafe(context.InstanceContext);
    FltReleaseFileNameInformation(context.FileNameInfo);

    return FLT_POSTOP_FINISHED_PROCESSING;
}

static NTSTATUS SetupFileContext(_Inout_ PDS_INITIALIZTION_CONTEXT Context) {
    DSR_ENTER(APC_LEVEL);
    PFLT_FILTER Filter = Context->FltObjects->Filter;
    PFLT_INSTANCE Instance = Context->FltObjects->Instance;
    PFILE_OBJECT FileObject = Context->FltObjects->FileObject;
    PDS_FILE_CONTEXT fileContext = NULL;
    if (!FltSupportsFileContextsEx(FileObject, Instance))
        DSR_RETURN(STATUS_FILE_CONTEXT_NOT_SUPPORTED);

    DSR_STATUS = FltGetFileContext(Instance, FileObject, &fileContext);
    if (DSR_STATUS == STATUS_NOT_FOUND) {
        DSR_ASSERT(FltAllocateContext(Filter, FLT_FILE_CONTEXT, sizeof(DS_FILE_CONTEXT), PagedPool, &fileContext));
        DSR_ASSERT(DsInitFileContext(fileContext));
        PDS_FILE_CONTEXT oldContext = NULL;
        DSR_STATUS = FltSetFileContext(Instance, FileObject, FLT_FILE_CONTEXT, fileContext, &oldContext);
        if (DSR_STATUS == STATUS_FLT_CONTEXT_ALREADY_DEFINED) {
            DsLogTrace("File context concurrent initialization detected.");
            FltReleaseContext(fileContext);
            fileContext = oldContext;
            DSR_STATUS = STATUS_SUCCESS;
        }
    }
    DSR_ASSERT_SUCCESS();

    Context->FileContext = fileContext;
    DSR_ERROR_HANDLER({
        FltReleaseContextSafe(fileContext);
    });
    return DSR_STATUS;
}

static NTSTATUS SetupStreamContext(_Inout_ PDS_INITIALIZTION_CONTEXT Context) {
    DSR_ENTER(APC_LEVEL);
    PFLT_FILTER Filter = Context->FltObjects->Filter;
    PFLT_INSTANCE Instance = Context->FltObjects->Instance;
    PFILE_OBJECT FileObject = Context->FltObjects->FileObject;
    PDS_STREAM_CONTEXT streamContext = NULL;
    if (!FltSupportsStreamContexts(FileObject))
        return STATUS_STREAM_CONTEXT_NOT_SUPPORTED;

    DSR_STATUS = FltGetStreamContext(Instance, FileObject, &streamContext);
    if (DSR_STATUS == STATUS_NOT_FOUND) {
        DSR_ASSERT(FltAllocateContext(Filter, FLT_STREAM_CONTEXT, sizeof(DS_STREAM_CONTEXT), PagedPool, &streamContext));
        DSR_ASSERT(DsInitStreamContext(streamContext));
        PDS_STREAM_CONTEXT oldContext = NULL;
        DSR_STATUS = FltSetStreamContext(Instance, FileObject, FLT_STREAM_CONTEXT, streamContext, &oldContext);
        if (DSR_STATUS == STATUS_FLT_CONTEXT_ALREADY_DEFINED) {
            DsLogTrace("Stream context concurrent initialization detected.");
            FltReleaseContext(streamContext);
            streamContext = oldContext;
            DSR_STATUS = STATUS_SUCCESS;
        }
    }
    DSR_ASSERT_SUCCESS();

    Context->StreamContext = streamContext;
    DSR_ERROR_HANDLER({
        FltReleaseContextSafe(streamContext);
    });
    return DSR_STATUS;
}
