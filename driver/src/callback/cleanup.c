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

#include "cleanup.h"
#include "context/stream.h"
#include "flow.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, DsPreCleanupCallback)
#endif

FLT_PREOP_CALLBACK_STATUS FLTAPI DsPreCleanupCallback(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _Outptr_result_maybenull_ PVOID *CompletionContext
) {
    UNREFERENCED_PARAMETER(Data);
    DSR_INIT(PASSIVE_LEVEL);
    *CompletionContext = NULL;

    PDS_STREAM_CONTEXT StreamContext = NULL;
    DSR_ASSERT(FltGetStreamContext(FltObjects->Instance, FltObjects->FileObject, &StreamContext));

    PDS_FILE_CONTEXT FileContext = StreamContext->FileContext;

    DsFlowLock(StreamContext);
    DsFlowDecrementHandles(StreamContext);
    DsLogTrace("Cleanup. Stream: %wZ. Count: %d.", &StreamContext->Data.FileName, StreamContext->Data.HandleCount);
    if (FileContext != NULL) {
        DsLogTrace("Cleanup. File: %wZ. Count: %d.", &FileContext->Data.FileName, FileContext->Data.HandleCount);
    }
    DsFlowRelease(StreamContext);

    DSR_CLEANUP_EMPTY();
    FltReleaseContextSafe(StreamContext);
    return FLT_PREOP_SUCCESS_NO_CALLBACK;
}
