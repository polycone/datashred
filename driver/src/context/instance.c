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
#pragma alloc_text(PAGE, DsCreateInstanceContext)
#pragma alloc_text(PAGE, DsCleanupInstanceContext)
#endif

NTSTATUS DsCreateInstanceContext(_In_ PCFLT_RELATED_OBJECTS FltObjects, _Out_ PDS_INSTANCE_CONTEXT *Context) {
    DSR_ENTER(PASSIVE_LEVEL);
    PDS_INSTANCE_CONTEXT context = NO_CONTEXT;
    DSR_ASSERT(FltAllocateContext(FltObjects->Filter, FLT_INSTANCE_CONTEXT, sizeof(DS_INSTANCE_CONTEXT), PagedPool, &context));

    DsInitUnicodeString(&context->VolumeGuid);
    DSR_ASSERT(DsGetVolumeGuidName(FltObjects->Volume, &context->VolumeGuid));
    DSR_ASSERT(DsGetVolumeProperties(FltObjects->Volume, &context->VolumeProperties));
    DSR_ASSERT(DsGetFileSystemProperties(FltObjects->Instance, &context->FileSystemProperties));

    DsLogTrace("Instance context created. [0x%p]", context);
    *Context = context;

    DSR_ERROR_HANDLER({
        if (context != NULL) {
            DsCleanupInstanceContext(context);
            FltReleaseContext(context);
        }
    });
    return DSR_STATUS;
}

VOID DsCleanupInstanceContext(_Inout_ PDS_INSTANCE_CONTEXT Context) {
    DsFreeUnicodeString(&Context->VolumeGuid);
}
