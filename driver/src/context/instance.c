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

#include "instance.h"
#include "util/string.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, DsInitInstanceContext)
#pragma alloc_text(PAGE, DsFreeInstanceContext)
#endif

NTSTATUS DsInitInstanceContext(_In_ PCFLT_RELATED_OBJECTS FltObjects, _Inout_ PDS_INSTANCE_CONTEXT *Context) {
    DSR_INIT;
    PDS_INSTANCE_CONTEXT context = EMPTY_CONTEXT;
    DSR_ASSERT(FltAllocateContext(FltObjects->Filter, FLT_INSTANCE_CONTEXT, sizeof(DS_INSTANCE_CONTEXT), PagedPool, &context));

    DsInitUnicodeString(&context->VolumeGuid);
    DSR_ASSERT(DsGetVolumeGuidName(FltObjects->Volume, &context->VolumeGuid));
    DSR_ASSERT(DsGetVolumeProperties(FltObjects->Volume, &context->VolumeProperties));

    DSR_ASSERT(FltSetInstanceContext(FltObjects->Instance, FLT_SET_CONTEXT_KEEP_IF_EXISTS, context, NULL));
    *Context = context;

    DSR_CLEANUP {
        DsFreeInstanceContext(context);
    };
    if (context != EMPTY_CONTEXT) {
        FltReleaseContext(context);
    }
    return DSR_STATUS;
}

VOID DsFreeInstanceContext(_In_ PDS_INSTANCE_CONTEXT Context) {
    DsFreeUnicodeString(&Context->VolumeGuid);
}
