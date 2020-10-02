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

    PDS_STREAM_CONTEXT streamContext = NULL;
    DSR_STATUS = FltGetStreamContext(FltObjects->Instance, FltObjects->FileObject, &streamContext);
    if (DSR_STATUS == STATUS_NOT_FOUND || DSR_STATUS == STATUS_NOT_SUPPORTED) {
        DSR_STATUS = STATUS_SUCCESS;
        DSR_CLEANUP();
    }
    DSR_CLEANUP_ON_FAIL();

    InterlockedDecrement(&streamContext->HandleCount);

    DsLogTrace("File closed: %wZ", &streamContext->FileName);

    if (streamContext->HandleCount == 0) {
        // TODO: Process a file when evaluated handle count reaches zero.
    }

    DSR_CLEANUP_EMPTY();
    if (streamContext != NULL) {
        FltReleaseContext(streamContext);
    }
    return FLT_PREOP_SUCCESS_NO_CALLBACK;
}
