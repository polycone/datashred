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
#pragma alloc_text(PAGE, DsCreateFileContext)
#pragma alloc_text(PAGE, DsCleanupFileContext)
#endif

NTSTATUS DsCreateFileContext(
    _In_ PFLT_FILTER Filter,
    _Outptr_ PDS_FILE_CONTEXT *Context
) {
    DSR_ENTER(APC_LEVEL);
    PDS_FILE_CONTEXT context = NO_CONTEXT;
    DSR_ASSERT(FltAllocateContext(Filter, FLT_FILE_CONTEXT, sizeof(DS_FILE_CONTEXT), PagedPool, &context));

    RtlZeroMemory(context, sizeof(DS_FILE_CONTEXT));
    FltInitializePushLock(&context->Lock);

    *Context = context;
    DSR_ERROR_HANDLER({
        FltReleaseContextSafe(context);
    });
    return DSR_STATUS;
}

VOID DsCleanupFileContext(_Inout_ PDS_FILE_CONTEXT Context) {
    FltDeletePushLock(&Context->Lock);
}
