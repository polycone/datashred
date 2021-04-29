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

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, DsInitFileContext)
#pragma alloc_text(PAGE, DsFreeFileContext)
#endif

NTSTATUS DsInitFileContext(_Inout_ PDS_FILE_CONTEXT FileContext) {
    DSR_INIT(APC_LEVEL);
    DSR_CLEANUP { }
    return DSR_STATUS;
}

VOID DsFreeFileContext(_In_ PDS_FILE_CONTEXT FileContext) {
}
