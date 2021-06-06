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

#define BUILD_NTSTATUS(severity, facility, code)    ((NTSTATUS)((severity << 30) | 0x20000000 | (facility << 16) | code))

/* Context related */

#define STATUS_FILE_CONTEXT_NOT_SUPPORTED           BUILD_NTSTATUS(STATUS_SEVERITY_ERROR, 0x001, 0x0001)
#define STATUS_STREAM_CONTEXT_NOT_SUPPORTED         BUILD_NTSTATUS(STATUS_SEVERITY_ERROR, 0x001, 0x0002)
#define STATUS_STREAM_LOCKED                        BUILD_NTSTATUS(STATUS_SEVERITY_ERROR, 0x001, 0x0003)

/* Pre-op statuses (order is the same as of FLT_PREOP_CALLBACK_STATUS) */

#define STATUS_PREOP_SUCCESS_WITH_CALLBACK          BUILD_NTSTATUS(STATUS_SEVERITY_SUCCESS, 0x002, 0x0000)
#define STATUS_PREOP_SUCCESS_NO_CALLBACK            BUILD_NTSTATUS(STATUS_SEVERITY_SUCCESS, 0x002, 0x0001)
#define STATUS_PREOP_PENDING                        BUILD_NTSTATUS(STATUS_SEVERITY_SUCCESS, 0x002, 0x0002)
#define STATUS_PREOP_DISALLOW_FASTIO                BUILD_NTSTATUS(STATUS_SEVERITY_SUCCESS, 0x002, 0x0003)
#define STATUS_PREOP_COMPLETE                       BUILD_NTSTATUS(STATUS_SEVERITY_SUCCESS, 0x002, 0x0004)
#define STATUS_PREOP_SYNCHRONIZE                    BUILD_NTSTATUS(STATUS_SEVERITY_SUCCESS, 0x002, 0x0005)
#define STATUS_PREOP_DISALLOW_FSFILTER_IO           BUILD_NTSTATUS(STATUS_SEVERITY_SUCCESS, 0x002, 0x0006)

/* General purpose */

#define STATUS_DISPOSITION_NON_DESTRUCTIVE          BUILD_NTSTATUS(STATUS_SEVERITY_SUCCESS, 0x003, 0x0000)
#define STATUS_DISPOSITION_FAILURE                  BUILD_NTSTATUS(STATUS_SEVERITY_ERROR, 0x003, 0x000)
