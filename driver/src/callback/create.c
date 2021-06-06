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
#include <memory.h>
#include <erase.h>
#include "create.h"
#include "status.h"

static NTSTATUS DsGetOrSetContext(
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ PCDS_CONTEXT_DESCRIPTOR Descriptor,
    _In_opt_ PVOID Parameters,
    _Out_ PFLT_CONTEXT *Context
);
static NTSTATUS DsSetupRelatedContexts(
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ PFLT_FILE_NAME_INFORMATION FileNameInformation,
    _Out_ PDS_STREAM_CONTEXT *Context
);
static NTSTATUS DsCreateFileContextWrapper(_In_opt_ PVOID Parameters, _Inout_ PFLT_CONTEXT *Context);
static NTSTATUS DsCreateStreamContextWrapper(_In_opt_ PVOID Parameters, _Inout_ PFLT_CONTEXT *Context);
static NTSTATUS DsProcessDisposition(
    PCFLT_RELATED_OBJECTS FltObjects,
    PFLT_PARAMETERS Parameters,
    PDS_CREATE_COMPLETION_CONTEXT CompletionContext
);
static NTSTATUS DsProcessDisposition(
    PCFLT_RELATED_OBJECTS FltObjects,
    PFLT_PARAMETERS Parameters,
    PDS_CREATE_COMPLETION_CONTEXT CompletionContext
);

__forceinline VOID FreeCompletionContext(PDS_CREATE_COMPLETION_CONTEXT CompletionContext);
__forceinline BOOLEAN DestructiveDisposition(ULONG CreateDisposition);
__forceinline BOOLEAN DispositionFailed(PFLT_CALLBACK_DATA Data, PDS_CREATE_COMPLETION_CONTEXT Context);

#define FreeCompletionContextSafe(context)          NULL_SAFE_CALL(FreeCompletionContext, context)

static CONST DS_CONTEXT_DESCRIPTOR DS_FILE_CONTEXT_DESCRIPTOR = { FltGetFileContext, FltSetFileContext, DsCreateFileContextWrapper };
static CONST DS_CONTEXT_DESCRIPTOR DS_STREAM_CONTEXT_DESCRIPTOR = { FltGetStreamContext, FltSetStreamContext, DsCreateStreamContextWrapper };

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, DsPreCreateCallback)
#pragma alloc_text(PAGE, DsPostCreateCallback)
#pragma alloc_text(PAGE, DsGetOrSetContext)
#pragma alloc_text(PAGE, DsSetupRelatedContexts)
#pragma alloc_text(PAGE, DsCreateFileContextWrapper)
#pragma alloc_text(PAGE, DsCreateStreamContextWrapper)
#pragma alloc_text(PAGE, DsCreateStreamContextWrapper)
#pragma alloc_text(PAGE, DsProcessDisposition)
#endif

FLT_PREOP_CALLBACK_STATUS DsPreCreateCallback(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _Out_ PVOID *CompletionContext
) {
    DSR_ENTER(PASSIVE_LEVEL);
    *CompletionContext = NULL;
    PDS_CREATE_COMPLETION_CONTEXT context = NULL;

    if (FlagOn(Data->Iopb->Parameters.Create.Options, FILE_DIRECTORY_FILE))
        DSR_RETURN(STATUS_PREOP_SUCCESS_NO_CALLBACK);
    if (FlagOn(Data->Iopb->OperationFlags, SL_OPEN_PAGING_FILE))
        DSR_RETURN(STATUS_PREOP_SUCCESS_NO_CALLBACK);
    if (FlagOn(Data->Iopb->TargetFileObject->Flags, FO_VOLUME_OPEN))
        DSR_RETURN(STATUS_PREOP_SUCCESS_NO_CALLBACK);

    DSR_ASSERT(DsMemAllocType(DS_CREATE_COMPLETION_CONTEXT, &context));

    DSR_ASSERT(FltGetFileNameInformation(Data, DS_FLT_FILE_NAME_OPENED_CACHED, &context->FileNameInformation));
    DSR_ASSERT(FltParseFileNameInformation(context->FileNameInformation));
    if (!DsIsDataStream(&context->FileNameInformation->Stream))
        DSR_RETURN(STATUS_PREOP_SUCCESS_NO_CALLBACK);

    DSR_ASSERT(DsProcessDisposition(FltObjects, &Data->Iopb->Parameters, context));

    *CompletionContext = context;

    DSR_STATUS = STATUS_PREOP_SYNCHRONIZE;
    DSR_ERROR_HANDLER({
        Data->IoStatus.Status = DSR_STATUS;
        DSR_STATUS = STATUS_PREOP_COMPLETE;
        FreeCompletionContextSafe(context);
    });

    ASSERT_FLT_PREOP_CALLBACK_STATUS(DSR_STATUS);
    return TO_FLT_PREOP_CALLBACK_STATUS(DSR_STATUS);
}

FLT_POSTOP_CALLBACK_STATUS DsPostCreateCallback(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ PVOID CompletionContext,
    _In_ FLT_POST_OPERATION_FLAGS Flags
) {
    DSR_ENTER(PASSIVE_LEVEL);
    PDS_CREATE_COMPLETION_CONTEXT context = (PDS_CREATE_COMPLETION_CONTEXT)CompletionContext;
    PDS_STREAM_CONTEXT streamContext = NULL;

    if (DispositionFailed(Data, context))
        DSR_ASSERT(STATUS_DISPOSITION_FAILURE);

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

    if (context->StreamContext == NULL)
        DSR_ASSERT(DsSetupRelatedContexts(FltObjects, context->FileNameInformation, &context->StreamContext));

    if (DsStreamAddHandle(context->StreamContext, Data) == STATUS_STREAM_LOCKED) {
        FltCancelFileOpen(FltObjects->Instance, FltObjects->FileObject);
        Data->IoStatus.Status = STATUS_ACCESS_DENIED;
        DSR_LEAVE();
    }

    DSR_ERROR_HANDLER({});
    FreeCompletionContextSafe(context);
    return FLT_POSTOP_FINISHED_PROCESSING;
}

static NTSTATUS DsProcessDisposition(
    PCFLT_RELATED_OBJECTS FltObjects,
    PFLT_PARAMETERS Parameters,
    PDS_CREATE_COMPLETION_CONTEXT Context
) {
    DSR_ENTER(PASSIVE_LEVEL);
    DS_FILE_DESCRIPTOR descriptor = EMPTY_FILE_DESCRIPTOR;
    ULONG disposition = Parameters->Create.Options >> 24;
    Context->DispositionStatus = STATUS_DISPOSITION_NON_DESTRUCTIVE;
    if (!DestructiveDisposition(disposition))
        DSR_RETURN(STATUS_SUCCESS);

    ULONG dispositionDesiredAccess = disposition == FILE_SUPERSEDE ? DELETE : GENERIC_WRITE;
    NTSTATUS status = DsCreateFile(
        FltObjects,
        &Context->FileNameInformation->Name,
        Parameters->Create.SecurityContext->DesiredAccess | dispositionDesiredAccess,
        Parameters->Create.ShareAccess,
        DS_FILE_FORCE_ACCESS_CHECK,
        &descriptor
    );
    if (disposition == FILE_SUPERSEDE && status == STATUS_OBJECT_NAME_NOT_FOUND)
        DSR_RETURN(STATUS_SUCCESS);

    if (!NT_SUCCESS(status)) {
        Context->DispositionStatus = status;
        DSR_RETURN(STATUS_SUCCESS);
    }

    FLT_RELATED_OBJECTS NewFltObjects = OverrideFileObject(FltObjects, descriptor.FileObject);
    DSR_ASSERT(DsSetupRelatedContexts(&NewFltObjects, Context->FileNameInformation, &Context->StreamContext));

    DSR_ASSERT(DsEraseFile());

    DSR_ERROR_HANDLER({
        FltReleaseContextSafe(Context->StreamContext);
    });
    DsCloseFile(&descriptor);
    return DSR_STATUS;
}

static NTSTATUS DsSetupRelatedContexts(
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ PFLT_FILE_NAME_INFORMATION FileNameInformation,
    _Out_ PDS_STREAM_CONTEXT *Context
) {
    DSR_ENTER(PASSIVE_LEVEL);
    PDS_INSTANCE_CONTEXT instanceContext = NULL;
    PDS_FILE_CONTEXT fileContext = NULL;
    PDS_STREAM_CONTEXT streamContext = NULL;

    if (!FltSupportsFileContextsEx(FltObjects->FileObject, FltObjects->Instance))
        DSR_ASSERT(STATUS_FILE_CONTEXT_NOT_SUPPORTED);
    if (!FltSupportsStreamContexts(FltObjects->FileObject))
        DSR_ASSERT(STATUS_STREAM_CONTEXT_NOT_SUPPORTED);

    DSR_ASSERT(FltGetInstanceContext(FltObjects->Instance, &instanceContext));

    DS_FILE_CONTEXT_PARAMETERS fileContextParams = {
        DEBUG_ONLY(.Magic = FILE_CONTEXT_PARAMETERS_MAGIC),
        .Filter = FltObjects->Filter
    };
    DSR_ASSERT(DsGetOrSetContext(FltObjects, &DS_FILE_CONTEXT_DESCRIPTOR, &fileContextParams, &fileContext));

    DS_STREAM_CONTEXT_PARAMETERS streamContextParams = {
        DEBUG_ONLY(.Magic = STREAM_CONTEXT_PARAMETERS_MAGIC),
        .Filter = FltObjects->Filter,
        .FileNameInfo = FileNameInformation,
        .FileContext = fileContext,
        .InstanceContext = instanceContext
    };
    DSR_ASSERT(DsGetOrSetContext(FltObjects, &DS_STREAM_CONTEXT_DESCRIPTOR, &streamContextParams, &streamContext));

    *Context = streamContext;
    DSR_ERROR_HANDLER({});
    FltReleaseContextSafe(instanceContext);
    FltReleaseContextSafe(fileContext);
    return DSR_STATUS;
}

static NTSTATUS DsGetOrSetContext(
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ PCDS_CONTEXT_DESCRIPTOR Descriptor,
    _In_opt_ PVOID Parameters,
    _Out_ PFLT_CONTEXT *Context
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
    DSR_ERROR_HANDLER({
        FltReleaseContextSafe(context);
    });
    return DSR_STATUS;
}

static NTSTATUS DsCreateFileContextWrapper(_In_opt_ PVOID Parameters, _Inout_ PFLT_CONTEXT *Context) {
    DSR_ENTER(APC_LEVEL);
    PDS_FILE_CONTEXT_PARAMETERS parameters = (PDS_FILE_CONTEXT_PARAMETERS)Parameters;
#ifdef DBG
    if (parameters == NULL || parameters->Magic != FILE_CONTEXT_PARAMETERS_MAGIC)
        DsRaiseAssertonFailure();
#endif
    DSR_ASSERT(DsCreateFileContext(parameters->Filter, (PDS_FILE_CONTEXT *)Context));
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
        parameters->Filter,
        parameters->InstanceContext,
        parameters->FileContext,
        parameters->FileNameInfo,
        (PDS_STREAM_CONTEXT *)Context
    ));
    DSR_ERROR_HANDLER({});
    return DSR_STATUS;
}

__forceinline BOOLEAN DestructiveDisposition(ULONG CreateDisposition) {
    return CreateDisposition == FILE_OVERWRITE ||
        CreateDisposition == FILE_OVERWRITE_IF ||
        CreateDisposition == FILE_SUPERSEDE;
}

__forceinline VOID FreeCompletionContext(PDS_CREATE_COMPLETION_CONTEXT CompletionContext) {
    FltReleaseFileNameInformationSafe(CompletionContext->FileNameInformation);
    FltReleaseContextSafe(CompletionContext->StreamContext);
    DsMemFree(CompletionContext);
}

__forceinline BOOLEAN DispositionFailed(PFLT_CALLBACK_DATA Data, PDS_CREATE_COMPLETION_CONTEXT Context) {
    return Context->DispositionStatus != STATUS_DISPOSITION_NON_DESTRUCTIVE &&
        NT_SUCCESS(Context->DispositionStatus) != NT_SUCCESS(Data->IoStatus.Status);
}
