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

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, DsInitializeStreamContext)
#pragma alloc_text(PAGE, DsFinalizeStreamContext)
#endif

NTSTATUS DsInitializeStreamContext(_In_ PDS_STREAM_CTX_INIT_DATA Data, _Out_ PDS_STREAM_CONTEXT Context) {
    DSR_ENTER(APC_LEVEL);
    RtlZeroMemory(Context, sizeof(DS_STREAM_CONTEXT));
    FltReferenceContext(Data->FileContext);
    Context->FileContext = Data->FileContext;
    Context->Default = DsIsDefaultStream(&Data->FileNameInfo->Stream);
#ifdef DBG
    DSR_ASSERT(DsCopyUnicodeString(&Context->Name, &Data->FileNameInfo->Name));
    DsLogTrace("Stream context initialized. [%wZ]", &Context->Name);
#endif
    DSR_ERROR_HANDLER({
        FltReleaseContextSafe(Data->FileContext);
    });
    return DSR_STATUS;
}

VOID DsFinalizeStreamContext(_Inout_ PDS_STREAM_CONTEXT Context) {
    FltReleaseContext(Context->FileContext);
#ifdef DBG
    DsFreeUnicodeString(&Context->Name);
#endif
}
