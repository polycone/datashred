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

#include "monitor.h"
#include "util/string.h"

NTSTATUS DsInitMonitorContext(
    _Inout_ PDS_MONITOR_CONTEXT MonitorContext,
    _In_ PUNICODE_STRING FileName,
    _In_ PDS_INSTANCE_CONTEXT InstanceContext
) {
    DSR_INIT(APC_LEVEL);
    DSR_ASSERT(DsCreateUnicodeString(&MonitorContext->Name, FileName->Length));
    RtlCopyUnicodeString(&MonitorContext->Name, FileName);
    MonitorContext->HandleCount = 0;
    MonitorContext->Flags = 0;
    MonitorContext->InstanceContext = InstanceContext;
    FltReferenceContext(InstanceContext);
    ASSERT(IS_ALIGNED(&MonitorContext->Lock, sizeof(void *)));
    FltInitializePushLock(&MonitorContext->Lock);
    DSR_CLEANUP_EMPTY();
    return DSR_STATUS;
}

VOID DsFreeMonitorContext(_In_ PDS_MONITOR_CONTEXT MonitorContext) {
    DsFreeUnicodeString(&MonitorContext->Name);
    FltReleaseContextSafe(MonitorContext->InstanceContext);
    FltDeletePushLock(&MonitorContext->Lock);
}
