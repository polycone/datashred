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

typedef NTSTATUS
(FLTAPI *FLT_GET_CONTEXT) (
    _In_ PFLT_INSTANCE Instance,
    _In_ PFILE_OBJECT FileObject,
    _Outptr_ PFLT_CONTEXT *Context
);

typedef NTSTATUS
(FLTAPI *FLT_SET_CONTEXT) (
    _In_ PFLT_INSTANCE Instance,
    _In_ PFILE_OBJECT FileObject,
    _In_ FLT_SET_CONTEXT_OPERATION Operation,
    _In_ PFLT_CONTEXT NewContext,
    _Outptr_opt_result_maybenull_ PFLT_CONTEXT *OldContext
);

typedef NTSTATUS
(FLTAPI *PDS_INIT_CONTEXT) (
    _In_opt_ PVOID Data,
    _Inout_ PFLT_CONTEXT Context
);

typedef struct _DS_CONTEXT_DESCRIPTOR {
    UNICODE_STRING Name;
    FLT_GET_CONTEXT Get;
    FLT_SET_CONTEXT Set;
    PDS_INIT_CONTEXT Initialize;
    FLT_CONTEXT_TYPE Type;
    SIZE_T Size;
} DS_CONTEXT_DESCRIPTOR, *PDS_CONTEXT_DESCRIPTOR;

static NTSTATUS DsSetupContext(
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ DS_CONTEXT_DESCRIPTOR const *Descriptor,
    _In_opt_ PVOID Data,
    _Outptr_ PFLT_CONTEXT *Context
);

static const DS_CONTEXT_DESCRIPTOR DsFileContextDescriptor = {
    RTL_CONSTANT_STRING(L"File"),
    FltGetFileContext,
    FltSetFileContext,
    DsInitializeFileContext,
    FLT_FILE_CONTEXT,
    sizeof(DS_FILE_CONTEXT)
};

static const DS_CONTEXT_DESCRIPTOR DsStreamContextDescriptor = {
    RTL_CONSTANT_STRING(L"Stream"),
    FltGetStreamContext,
    FltSetStreamContext,
    DsInitializeStreamContext,
    FLT_STREAM_CONTEXT,
    sizeof(DS_STREAM_CONTEXT)
};

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, DsPreCreateCallback)
#pragma alloc_text(PAGE, DsPostCreateCallback)
#pragma alloc_text(PAGE, DsSetupContext)
#endif

static NTSTATUS DsSetupContext(
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ DS_CONTEXT_DESCRIPTOR const *Descriptor,
    _In_opt_ PVOID Data,
    _Outptr_ PFLT_CONTEXT *Context
) {
    DSR_ENTER(APC_LEVEL);
    PFLT_CONTEXT context = NULL_CONTEXT;
    DSR_STATUS = Descriptor->Get(FltObjects->Instance, FltObjects->FileObject, &context);
    if (DSR_STATUS == STATUS_NOT_FOUND) {
        DSR_ASSERT(FltAllocateContext(FltObjects->Filter, Descriptor->Type, Descriptor->Size, PagedPool, &context));
        DSR_ASSERT(Descriptor->Initialize(Data, context));
        PFLT_CONTEXT previousContext = NULL_CONTEXT;
        DSR_STATUS = Descriptor->Set(FltObjects->Instance, FltObjects->FileObject, FLT_SET_CONTEXT_KEEP_IF_EXISTS, context, &previousContext);
        if (DSR_STATUS == STATUS_FLT_CONTEXT_ALREADY_DEFINED) {
            DsLogInfo("%wZ context concurrent init. [0x%p]", Descriptor->Name, previousContext);
            FltReleaseContext(context);
            context = previousContext;
            *Context = context;
            DSR_STATUS = STATUS_SUCCESS;
        }
    }
    DSR_ASSERT_SUCCESS();
    *Context = context;
    DSR_ERROR_HANDLER({});
    FltReleaseContextSafe(context);
    return DSR_STATUS;
}

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

    DSR_ASSERT(DsSetupContext(FltObjects, &DsFileContextDescriptor, NULL, &fileContext));

    DS_STREAM_CTX_INIT_DATA streamContextInitData = { fileContext, fileNameInfo };
    DSR_ASSERT(DsSetupContext(FltObjects, &DsStreamContextDescriptor, &streamContextInitData, &streamContext));

    DSR_ERROR_HANDLER({ });

    FltReleaseContextSafe(instanceContext);
    FltReleaseFileNameInformation(fileNameInfo);
    return FLT_POSTOP_FINISHED_PROCESSING;
}
