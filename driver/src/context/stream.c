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

#include <context.h>
#include <dsr.h>
#include <file.h>

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, DsCreateStreamContext)
#pragma alloc_text(PAGE, DsCleanupStreamContext)
#pragma alloc_text(PAGE, DsStreamAddHandle)
#pragma alloc_text(PAGE, DsStreamRemoveHandle)
#endif

NTSTATUS DsCreateStreamContext(
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ PDS_INSTANCE_CONTEXT InstanceContext,
    _In_ PDS_FILE_CONTEXT FileContext,
    _In_ PFLT_FILE_NAME_INFORMATION FileNameInformation,
    _Outptr_ PDS_STREAM_CONTEXT *Context
) {
    DSR_ENTER(APC_LEVEL);
    PDS_STREAM_CONTEXT context = NO_CONTEXT;
    DSR_ASSERT(FltAllocateContext(FltObjects->Filter, FLT_STREAM_CONTEXT, sizeof(DS_STREAM_CONTEXT), PagedPool, &context));

    RtlZeroMemory(context, sizeof(DS_STREAM_CONTEXT));
    FltReferenceContext(FileContext);
    FltReferenceContext(InstanceContext);
    context->FileContext = FileContext;
    context->InstanceContext = InstanceContext;
    context->AlternateStream = DsIsAlternateStream(&FileNameInformation->Stream);

#ifdef DBG
    DSR_ASSERT(DsCopyUnicodeString(&context->Name, &FileNameInformation->Name));
    DsLogTrace("Stream context created. [%wZ]", &context->Name);
#endif

    *Context = context;
    DSR_ERROR_HANDLER({
        FltReleaseContextSafe(FileContext);
        FltReleaseContextSafe(InstanceContext);
        FltReleaseContextSafe(context);
    });
    return DSR_STATUS;
}

VOID DsCleanupStreamContext(_Inout_ PDS_STREAM_CONTEXT Context) {
    FltReleaseContext(Context->FileContext);
    FltReleaseContext(Context->InstanceContext);
#ifdef DBG
    DsFreeUnicodeString(&Context->Name);
#endif
}

NTSTATUS DsStreamAddHandle(_In_ PDS_STREAM_CONTEXT StreamContext, _In_ PFLT_CALLBACK_DATA Data) {
    NTSTATUS result = STATUS_SUCCESS;
    PDS_FILE_CONTEXT FileContext = StreamContext->FileContext;
    BOOLEAN deleteOnClose = FlagOn(Data->Iopb->Parameters.Create.Options, FILE_DELETE_ON_CLOSE) > 0;
    DSR_CRITICAL_EXCLUSIVE(&FileContext->Lock, {
        if (StreamContext->Locked || FileContext->Locked) {
            result = STATUS_STREAM_LOCKED;
            DSR_CRITICAL_LEAVE();
        }
        StreamContext->HandleCount++;
        StreamContext->DeleteOnClose = deleteOnClose;
        FileContext->HandleCount++;
        if (!StreamContext->AlternateStream)
            FileContext->DeleteOnClose = deleteOnClose;
        DsLogTrace("Handle added. [0x%p]", StreamContext);
    });
    return result;
}

VOID DsStreamRemoveHandle(_In_ PDS_STREAM_CONTEXT StreamContext) {
    PDS_FILE_CONTEXT FileContext = StreamContext->FileContext;
    DSR_CRITICAL_EXCLUSIVE(&FileContext->Lock, {
        StreamContext->HandleCount--;
        FileContext->HandleCount--;
        DsLogTrace("Handle removed. [0x%p]", StreamContext);
    });
}
