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
#include <fltKernel.h>
#include <dontuse.h>
#pragma prefast(disable:__WARNING_ENCODE_MEMBER_FUNCTION_POINTER, "Not valid for kernel mode drivers")

#define FLTFL_NONE      0
#define NO_CALLBACK     NULL
#define NO_CONTEXT      NULL

#ifdef DBG
#define DsDbgPrint(level, format, ...) DbgPrintEx(DPFLTR_IHVDRIVER_ID, level, format, __VA_ARGS__)
#define DsLogInfo(format, ...) DsDbgPrint(DPFLTR_INFO_LEVEL, DRIVER_NAME "!" __FUNCTION__ ": " format "\n", __VA_ARGS__)
#define DsLogTrace(format, ...) DsDbgPrint(DPFLTR_TRACE_LEVEL, DRIVER_NAME "!" __FUNCTION__ ": " format "\n", __VA_ARGS__)
#else
#define DsDbgPrint __noop
#define DsLogInfo __noop
#define DsLogTrace __noop
#endif
