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
    DSR_ASSERT(
        FltGetStreamContext(FltObjects->Instance, FltObjects->FileObject, &StreamContext),
        DSR_SUPPRESS(STATUS_NOT_FOUND)
        DSR_SUPPRESS(STATUS_NOT_SUPPORTED)
    );
    PDS_FILE_CONTEXT FileContext = StreamContext->FileContext;

    InterlockedDecrement(&StreamContext->HandleCount);
    DsLogTrace("Cleanup. Stream: %wZ. Count: %d.", &StreamContext->FileName, StreamContext->HandleCount);
    if (FileContext != NULL) {
        InterlockedDecrement(&StreamContext->FileContext->HandleCount);
        DsLogTrace("Cleanup. File: %wZ. Count: %d.", &FileContext->FileName, FileContext->HandleCount);
    }

    BOOLEAN eraseStream = StreamContext->HandleCount == 0 && StreamContext->DeleteOnClose;
    if (FileContext != NULL) {
        BOOLEAN eraseFile = FileContext->HandleCount == 0 && FileContext->DeleteOnClose;
        if (FileContext->DeleteOnClose && StreamContext->DefaultStream)
            eraseStream = FALSE;
        if (eraseFile) {
            DsLogTrace("Erase. File: %wZ.", &FileContext->FileName);
            // TODO: Erase the entire file and streams
            eraseStream = FALSE;
        }
    }

    if (eraseStream) {
        DsLogTrace("Erase. Stream: %wZ.", &StreamContext->FileName);
        // TODO: Erase a file stream
    }

    DSR_CLEANUP_EMPTY();
    FltReleaseContextSafe(StreamContext);
    return FLT_PREOP_SUCCESS_NO_CALLBACK;
}
