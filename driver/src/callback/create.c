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
#include <file.h>
#include "create.h"

static NTSTATUS DsCreateContext(
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ DS_CONTEXT_DESCRIPTOR *Descriptor,
    _In_opt_ PVOID Parameters,
    _Outptr_ PFLT_CONTEXT *Context
);
static NTSTATUS DsCreateFileContextWrapper(_In_opt_ PVOID Parameters, _Inout_ PFLT_CONTEXT Context);
static NTSTATUS DsCreateStreamContextWrapper(_In_opt_ PVOID Parameters, _Inout_ PFLT_CONTEXT Context);

static DS_CONTEXT_DESCRIPTOR DsFileContextDescriptor = { FltGetFileContext, FltSetFileContext, DsCreateFileContextWrapper };
static DS_CONTEXT_DESCRIPTOR DsStreamContextDescriptor = { FltGetStreamContext, FltSetStreamContext, DsCreateStreamContextWrapper };

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, DsPreCreateCallback)
#pragma alloc_text(PAGE, DsPostCreateCallback)
#pragma alloc_text(PAGE, DsCreateContext)
#pragma alloc_text(PAGE, DsCreateFileContextWrapper)
#pragma alloc_text(PAGE, DsCreateStreamContextWrapper)
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
    if (disposition == FILE_SUPERSEDE || disposition == FILE_OVERWRITE || disposition == FILE_OVERWRITE_IF) {
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
    PFLT_FILE_NAME_INFORMATION fileNameInfo = (PFLT_FILE_NAME_INFORMATION)CompletionContext;
    PDS_INSTANCE_CONTEXT instanceContext = NULL;
    PDS_FILE_CONTEXT fileContext = NULL;
    PDS_STREAM_CONTEXT streamContext = NULL;

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

    DSR_ASSERT(FltParseFileNameInformation(fileNameInfo));
    if (!DsIsDataStream(&fileNameInfo->Stream))
        DSR_LEAVE();

    DSR_ASSERT(FltGetInstanceContext(FltObjects->Instance, &instanceContext));

    if (!FltSupportsFileContextsEx(FltObjects->FileObject, FltObjects->Instance))
        DSR_ASSERT(STATUS_FILE_CONTEXT_NOT_SUPPORTED);
    if (!FltSupportsStreamContexts(FltObjects->FileObject))
        DSR_ASSERT(STATUS_STREAM_CONTEXT_NOT_SUPPORTED);

    DSR_ASSERT(DsCreateContext(
        FltObjects,
        &DsFileContextDescriptor,
        &INLINE_TYPE(DS_FILE_CONTEXT_PARAMETERS,
            DEBUG_ONLY(.Magic = FILE_CONTEXT_PARAMETERS_MAGIC),
            .FltObjects = FltObjects
        ),
        &fileContext
    ));
    DSR_ASSERT(DsCreateContext(
        FltObjects,
        &DsStreamContextDescriptor,
        &INLINE_TYPE(DS_STREAM_CONTEXT_PARAMETERS,
            DEBUG_ONLY(.Magic = STREAM_CONTEXT_PARAMETERS_MAGIC),
            .FltObjects = FltObjects,
            .FileNameInfo = fileNameInfo,
            .FileContext = fileContext
        ),
        &streamContext
    ));

    DSR_ERROR_HANDLER({});

    FltReleaseContextSafe(instanceContext);
    FltReleaseFileNameInformation(fileNameInfo);
    return FLT_POSTOP_FINISHED_PROCESSING;
}

static NTSTATUS DsCreateContext(
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ DS_CONTEXT_DESCRIPTOR *Descriptor,
    _In_opt_ PVOID Parameters,
    _Outptr_ PFLT_CONTEXT *Context
) {
    DSR_ENTER(APC_LEVEL);
    PFLT_CONTEXT context = NULL_CONTEXT;
    DSR_STATUS = Descriptor->Get(FltObjects->Instance, FltObjects->FileObject, &context);
    if (DSR_STATUS == STATUS_NOT_FOUND) {
        DSR_ASSERT(Descriptor->Create(Parameters, &context));
        PFLT_CONTEXT previousContext = NULL_CONTEXT;
        DSR_STATUS = Descriptor->Set(FltObjects->Instance, FltObjects->FileObject, FLT_SET_CONTEXT_KEEP_IF_EXISTS, context, &previousContext);
        if (DSR_STATUS == STATUS_FLT_CONTEXT_ALREADY_DEFINED) {
            DsLogInfo("Context concurrent initializtion detected. [0x%p]", previousContext);
            FltReleaseContext(context);
            context = previousContext;
            DSR_STATUS = STATUS_SUCCESS;
        }
    }
    DSR_ASSERT_SUCCESS();
    *Context = context;
    DSR_ERROR_HANDLER({});
    FltReleaseContextSafe(context);
    return DSR_STATUS;
}

static NTSTATUS DsCreateFileContextWrapper(_In_opt_ PVOID Parameters, _Inout_ PFLT_CONTEXT *Context) {
    DSR_ENTER(APC_LEVEL);
    PDS_FILE_CONTEXT_PARAMETERS parameters = (PDS_FILE_CONTEXT_PARAMETERS)Parameters;
#ifdef DBG
    if (parameters == NULL || parameters->Magic != FILE_CONTEXT_PARAMETERS_MAGIC)
        DsRaiseAssertonFailure();
#endif
    DSR_ASSERT(DsCreateFileContext(parameters->FltObjects, (PDS_FILE_CONTEXT *)Context));
    DSR_ERROR_HANDLER({});
    return DSR_STATUS;
}

static NTSTATUS DsCreateStreamContextWrapper(_In_opt_ PVOID Parameters, _Inout_ PFLT_CONTEXT *Context) {
    DSR_ENTER(APC_LEVEL);
    PDS_STREAM_CONTEXT_PARAMETERS parameters = (PDS_STREAM_CONTEXT_PARAMETERS)Parameters;
#ifdef DBG
    if (parameters == NULL || parameters->Magic != STREAM_CONTEXT_PARAMETERS_MAGIC)
        DsRaiseAssertonFailure();
#endif
    DSR_ASSERT(DsCreateStreamContext(
        parameters->FltObjects,
        parameters->FileContext,
        parameters->FileNameInfo,
        (PDS_STREAM_CONTEXT *)Context
    ));
    DSR_ERROR_HANDLER({});
    return DSR_STATUS;
}
