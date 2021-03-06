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

#pragma once
#include <ntdef.h>
#include <sal.h>

#define TO_POINTER(address)                         (PVOID)(address)
#define TO_ADDRESS(pointer)                         (SIZE_T)(pointer)
#define POINTER_AT_OFFSET(pointer, offset)          TO_POINTER(TO_ADDRESS(pointer) + (offset))

#define DS_MEMORY_INIT_ZERO                         0x00000001

NTSTATUS DsMemAllocEx(SIZE_T Size, ULONG Flags, _Out_ PVOID *Pointer);
VOID DsMemFree(_In_ PVOID Pointer);

#define DsMemAlloc(size, pointer)                   DsMemAllocEx(size, 0, pointer)

#define DsMemAllocType(type, pointer)               DsMemAllocEx(sizeof(type), DS_MEMORY_INIT_ZERO, pointer)
#define DsMemFreeSafe(pointer)                      \
    if (pointer != NULL)                            \
        DsMemFree(pointer);
