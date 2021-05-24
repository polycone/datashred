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
#endif

NTSTATUS DsCreateStreamContext(
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _In_ PDS_FILE_CONTEXT FileContext,
    _In_ PFLT_FILE_NAME_INFORMATION FileNameInformation,
    _Outptr_ PDS_STREAM_CONTEXT *Context
) {
    DSR_ENTER(APC_LEVEL);
    PDS_STREAM_CONTEXT context = NO_CONTEXT;
    DSR_ASSERT(FltAllocateContext(FltObjects->Filter, FLT_STREAM_CONTEXT, sizeof(DS_STREAM_CONTEXT), PagedPool, &context));

    RtlZeroMemory(context, sizeof(DS_STREAM_CONTEXT));
    FltReferenceContext(FileContext);
    context->FileContext = FileContext;
    context->Main = DsIsMainStream(&FileNameInformation->Stream);

#ifdef DBG
    DSR_ASSERT(DsCopyUnicodeString(&context->Name, &FileNameInformation->Name));
    DsLogTrace("Stream context created. [%wZ]", &context->Name);
#endif

    *Context = context;
    DSR_ERROR_HANDLER({
        FltReleaseContextSafe(FileContext);
        FltReleaseContextSafe(context);
    });
    return DSR_STATUS;
}

VOID DsCleanupStreamContext(_Inout_ PDS_STREAM_CONTEXT Context) {
    FltReleaseContext(Context->FileContext);
#ifdef DBG
    DsFreeUnicodeString(&Context->Name);
#endif
}
