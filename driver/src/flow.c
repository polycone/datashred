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

VOID DsFlowSetFlags(_In_ PDS_STREAM_CONTEXT Context, _In_ DS_CONTEXT_FLAGS Flags) {
    SetFlag(Context->Data.Flags, Flags);
    if (FlagsOn(Context->Data.Flags, DSCF_DEFAULT | DSCF_USE_FILE_CONTEXT)) {
        SetFlag(Context->FileContext->Data.Flags, Flags);
    }
}

VOID DsFlowClearFlags(_In_ PDS_STREAM_CONTEXT Context, _In_ DS_CONTEXT_FLAGS Flags) {
    ClearFlag(Context->Data.Flags, Flags);
    if (FlagsOn(Context->Data.Flags, DSCF_DEFAULT | DSCF_USE_FILE_CONTEXT)) {
        ClearFlag(Context->FileContext->Data.Flags, Flags);
    }
}

VOID DsFlowIncrementHandles(_In_ PDS_STREAM_CONTEXT Context) {
    ++Context->Data.HandleCount;
    if (FlagsOn(Context->Data.Flags, DSCF_USE_FILE_CONTEXT)) {
        ++Context->FileContext->Data.HandleCount;
    }
}

VOID DsFlowDecrementHandles(_In_ PDS_STREAM_CONTEXT Context) {
    --Context->Data.HandleCount;
    if (FlagsOn(Context->Data.Flags, DSCF_USE_FILE_CONTEXT)) {
        --Context->FileContext->Data.HandleCount;
    }
}

VOID DsFlowLock(_In_ PDS_STREAM_CONTEXT Context) {
    PEX_PUSH_LOCK PushLock = &Context->Data.Lock;
    if (FlagOn(Context->Data.Flags, DSCF_USE_FILE_CONTEXT)) {
        PushLock = &Context->FileContext->Data.Lock;
    }
    FltAcquirePushLockExclusive(PushLock);
}

VOID DsFlowRelease(_In_ PDS_STREAM_CONTEXT Context) {
    PEX_PUSH_LOCK PushLock = &Context->Data.Lock;
    if (FlagOn(Context->Data.Flags, DSCF_USE_FILE_CONTEXT)) {
        PushLock = &Context->FileContext->Data.Lock;
    }
    FltReleasePushLock(PushLock);
}
