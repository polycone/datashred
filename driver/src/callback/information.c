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

FLT_PREOP_CALLBACK_STATUS FLTAPI DsPreSetInformationCallback(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _Out_ PVOID *CompletionContext
) {
    DSR_INIT(PASSIVE_LEVEL);
    *CompletionContext = NULL;
    if (FlagOn(Data->Flags, FLTFL_CALLBACK_DATA_FAST_IO_OPERATION))
        return FLT_PREOP_DISALLOW_FASTIO;

    PDS_STREAM_CONTEXT StreamContext = NULL;
    DSR_STATUS = FltGetStreamContext(FltObjects->Instance, FltObjects->FileObject, &StreamContext);
    if (DSR_STATUS == STATUS_NOT_FOUND || DSR_STATUS == STATUS_NOT_SUPPORTED) {
        DSR_STATUS = STATUS_SUCCESS;
        DSR_CLEANUP();
    }
    DSR_CLEANUP_ON_FAIL();

    DSR_CLEANUP_EMPTY();

    FltReleaseContextSafe(StreamContext);
    return FLT_PREOP_SUCCESS_NO_CALLBACK;
}
