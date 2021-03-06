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

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, DsPreCleanupCallback)
#endif

FLT_PREOP_CALLBACK_STATUS FLTAPI DsPreCleanupCallback(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _Outptr_result_maybenull_ PVOID *CompletionContext
) {
    DSR_ENTER(PASSIVE_LEVEL);
    *CompletionContext = NULL;
    PDS_STREAM_CONTEXT StreamContext = NULL;
    DSR_STATUS = FltGetStreamContext(FltObjects->Instance, FltObjects->FileObject, &StreamContext);
    if (DSR_STATUS == STATUS_NOT_FOUND || DSR_STATUS == STATUS_NOT_SUPPORTED)
        DSR_RETURN(STATUS_SUCCESS);

    DsStreamRemoveHandle(StreamContext);

    DSR_ERROR_HANDLER({});
    FltReleaseContextSafe(StreamContext);
    return FLT_PREOP_SUCCESS_NO_CALLBACK;
}
