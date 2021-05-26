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

#define BUILD_NTSTATUS(severity, facility, code) ((NTSTATUS)((severity << 30) | 0x20000000 | (facility << 16) | code))

// Context related
#define STATUS_FILE_CONTEXT_NOT_SUPPORTED           BUILD_NTSTATUS(STATUS_SEVERITY_ERROR, 0x001, 0x0001)
#define STATUS_STREAM_CONTEXT_NOT_SUPPORTED         BUILD_NTSTATUS(STATUS_SEVERITY_ERROR, 0x001, 0x0002)
#define STATUS_STREAM_LOCKED                        BUILD_NTSTATUS(STATUS_SEVERITY_ERROR, 0x001, 0x0003)
