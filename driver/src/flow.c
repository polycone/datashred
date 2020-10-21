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

#include "flow.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, DsFlowSetFlags)
#pragma alloc_text(PAGE, DsFlowClearFlags)
#pragma alloc_text(PAGE, DsFlowIncrementHandles)
#pragma alloc_text(PAGE, DsFlowDecrementHandles)
#pragma alloc_text(PAGE, DsFlowLock)
#pragma alloc_text(PAGE, DsFlowRelease)
#endif

VOID DsFlowSetFlags(_In_ PDS_STREAM_CONTEXT Context, _In_ DS_MONITOR_FLAGS Flags) {
    SetFlag(Context->MonitorContext.Flags, Flags);
    if (Context->FileContext != NULL && FlagsOn(Context->MonitorContext.Flags, DS_MONITOR_FILE_DEFAULT_STREAM)) {
        SetFlag(Context->FileContext->MonitorContext.Flags, Flags);
    }
}

VOID DsFlowClearFlags(_In_ PDS_STREAM_CONTEXT Context, _In_ DS_MONITOR_FLAGS Flags) {
    ClearFlag(Context->MonitorContext.Flags, Flags);
    if (Context->FileContext != NULL && FlagsOn(Context->MonitorContext.Flags, DS_MONITOR_FILE_DEFAULT_STREAM)) {
        ClearFlag(Context->FileContext->MonitorContext.Flags, Flags);
    }
}

VOID DsFlowIncrementHandles(_In_ PDS_STREAM_CONTEXT Context) {
    ++Context->MonitorContext.HandleCount;
    if (Context->FileContext != NULL) {
        ++Context->FileContext->MonitorContext.HandleCount;
    }
}

VOID DsFlowDecrementHandles(_In_ PDS_STREAM_CONTEXT Context) {
    --Context->MonitorContext.HandleCount;
    if (Context->FileContext != NULL) {
        --Context->FileContext->MonitorContext.HandleCount;
    }
}

VOID DsFlowLock(_In_ PDS_STREAM_CONTEXT Context) {
    PEX_PUSH_LOCK PushLock = &Context->MonitorContext.Lock;
    if (Context->FileContext != NULL) {
        PushLock = &Context->FileContext->MonitorContext.Lock;
    }
    FltAcquirePushLockExclusive(PushLock);
}

VOID DsFlowRelease(_In_ PDS_STREAM_CONTEXT Context) {
    PEX_PUSH_LOCK PushLock = &Context->MonitorContext.Lock;
    if (Context->FileContext != NULL) {
        PushLock = &Context->FileContext->MonitorContext.Lock;
    }
    FltReleasePushLock(PushLock);
}
