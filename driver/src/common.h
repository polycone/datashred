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

#define EMPTY_FLAGS        0
#define EMPTY_CALLBACK     NULL
#define EMPTY_CONTEXT      NULL

/* Kernel debug messaging */
#ifdef DBG

#define DsDbgPrint(level, format, ...)  DbgPrintEx(DPFLTR_IHVDRIVER_ID, level, format, __VA_ARGS__)

#define LOG_PREFIX                      DRIVER_NAME "!" __FUNCTION__ ": "
#define DsLogInfo(format, ...)          DsDbgPrint(DPFLTR_INFO_LEVEL, LOG_PREFIX format "\n", __VA_ARGS__)
#define DsLogTrace(format, ...)         DsDbgPrint(DPFLTR_TRACE_LEVEL, LOG_PREFIX format "\n", __VA_ARGS__)

#else

#define DsDbgPrint __noop
#define DsLogInfo __noop
#define DsLogTrace __noop

#endif

/* Macro helpers */
#define __VA_PASS__(x) x
#define __GET_MACRO_2__(_1, _2, name, ...) name

/* Routine definitions */
#define DSR_INIT \
    PAGED_CODE(); \
    NTSTATUS status = STATUS_SUCCESS;

#define __DSR_ASSERT_WITH_SUPPRESS(op, s) \
    status = op; \
    s; \
    if (!NT_SUCCESS(status)) \
        goto cleanup;

#define __DSR_ASSERT(op) __DSR_ASSERT_WITH_SUPPRESS(op, )

#define DSR_ASSERT(...) \
    __VA_PASS__(__GET_MACRO_2__( \
                __VA_ARGS__, \
                __DSR_ASSERT_WITH_SUPPRESS, \
                __DSR_ASSERT \
                )(__VA_ARGS__) \
    )

#define DSR_SUPPRESS(s) \
    if (status == s) \
    status = STATUS_SUCCESS;

#define DSR_CLEANUP \
    cleanup: \
    if (!NT_SUCCESS(status))

#define DSR_STATUS                          status
