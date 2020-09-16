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

#include "memory.h"

#define DEFAULT_PAGED_POOL_TAG      'pPsD'

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, DsMemAlloc)
#pragma alloc_text(PAGE, DsMemFree)
#endif

NTSTATUS DsMemAlloc(_In_ SIZE_T Size, _Out_ PVOID *Pointer) {
    PVOID pointer = ExAllocatePoolWithTag(PagedPool, Size, DEFAULT_PAGED_POOL_TAG);
    if (pointer == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    *Pointer = pointer;
    return STATUS_SUCCESS;
}

VOID DsMemFree(_In_ PVOID Pointer) {
    ExFreePoolWithTag(Pointer, DEFAULT_PAGED_POOL_TAG);
}