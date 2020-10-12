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

#include "information.h"
#include "context/stream.h"
#include "flow.h"

static VOID ProcessFileDispositionInformationEx(
    _In_ PDS_STREAM_CONTEXT StreamContext,
    _In_ PFILE_DISPOSITION_INFORMATION_EX Disposition
);

FLT_PREOP_CALLBACK_STATUS FLTAPI DsPreSetInformationCallback(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _Outptr_result_maybenull_ PVOID *CompletionContext
) {
    DSR_INIT(PASSIVE_LEVEL);
    *CompletionContext = NULL;
    if (FlagOn(Data->Flags, FLTFL_CALLBACK_DATA_FAST_IO_OPERATION))
        return FLT_PREOP_DISALLOW_FASTIO;

    FLT_PREOP_CALLBACK_STATUS callbackStatus = FLT_PREOP_SUCCESS_WITH_CALLBACK;
    PDS_STREAM_CONTEXT StreamContext = NULL;
    DSR_STATUS = FltGetStreamContext(FltObjects->Instance, FltObjects->FileObject, &StreamContext);
    if (DSR_STATUS == STATUS_NOT_FOUND || DSR_STATUS == STATUS_NOT_SUPPORTED) {
        DSR_STATUS = STATUS_SUCCESS;
        DSR_CLEANUP();
    }
    DSR_CLEANUP_ON_FAIL();

    switch (Data->Iopb->Parameters.SetFileInformation.FileInformationClass) {
        case FileDispositionInformation:
        case FileDispositionInformationEx:
            break;
        default:
            callbackStatus = FLT_PREOP_SUCCESS_NO_CALLBACK;
            DSR_CLEANUP();
    }

    *CompletionContext = StreamContext;

    DSR_CLEANUP_START();
    Data->IoStatus.Status = DSR_STATUS;
    callbackStatus = FLT_PREOP_COMPLETE;
    DSR_CLEANUP_END();
    if (callbackStatus != FLT_PREOP_SUCCESS_WITH_CALLBACK) {
        FltReleaseContextSafe(StreamContext);
    }
    return callbackStatus;
}

FLT_POSTOP_CALLBACK_STATUS DsPostSetInformationCallback(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ PVOID CompletionContext,
    _In_ FLT_POST_OPERATION_FLAGS Flags
) {
    UNREFERENCED_PARAMETER(FltObjects);
    UNREFERENCED_PARAMETER(Flags);
    DSR_INIT(PASSIVE_LEVEL);
    PDS_STREAM_CONTEXT StreamContext = (PDS_STREAM_CONTEXT)CompletionContext;

    if (!NT_SUCCESS(Data->IoStatus.Status))
        DSR_CLEANUP();

    switch (Data->Iopb->Parameters.SetFileInformation.FileInformationClass)
    {
        case FileDispositionInformationEx: {
            ProcessFileDispositionInformationEx(
                StreamContext,
                (PFILE_DISPOSITION_INFORMATION_EX)Data->Iopb->Parameters.SetFileInformation.InfoBuffer
            );
            break;
        }
    }

    DSR_CLEANUP_EMPTY();

    FltReleaseContextSafe(StreamContext);
    return FLT_POSTOP_FINISHED_PROCESSING;
}

static VOID ProcessFileDispositionInformationEx(
    _In_ PDS_STREAM_CONTEXT StreamContext,
    _In_ PFILE_DISPOSITION_INFORMATION_EX Disposition
) {
    if (FlagOn(Disposition->Flags, FILE_DISPOSITION_ON_CLOSE)) {
        if (FlagOn(Disposition->Flags, FILE_DISPOSITION_DELETE))
            DsFlowSetFlags(StreamContext, DSCF_DELETE_ON_CLOSE);
        else
            DsFlowClearFlags(StreamContext, DSCF_DELETE_ON_CLOSE);
    }
}
