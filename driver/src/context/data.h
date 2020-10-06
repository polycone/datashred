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

#pragma once
#include "common.h"

/* Context data flags */
#define DS_CONTEXT_FLAGS ULONG

#define DSCF_DEFAULT                0x00000001
#define DSCF_DELETE_ON_CLOSE        0x00000002
#define DSCF_USE_FILE_CONTEXT       0x00000004
#define DSCF_PUSH_LOCK_ACTIVE       0x00000008

#ifdef _WIN64
#define PUSH_LOCK_ALIGN __declspec(align(8))
#else
#define PUSH_LOCK_ALIGN __declspec(align(4))
#endif // _WIN64

typedef struct _DS_CONTEXT_DATA {
    UNICODE_STRING FileName;
    volatile LONG HandleCount;
    DS_CONTEXT_FLAGS Flags;
    PUSH_LOCK_ALIGN EX_PUSH_LOCK Lock;
} DS_CONTEXT_DATA, *PDS_CONTEXT_DATA;
