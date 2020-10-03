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
#pragma warning(disable:4214) // Enable custom bit fields type
#pragma warning(disable:4201) // Enable nameless structs/unions

#define EMPTY_FLAGS        0
#define EMPTY_CALLBACK     NULL
#define EMPTY_CONTEXT      NULL

/* Kernel debug messaging */
#ifdef DBG

#define DsDbgPrint(level, format, ...)  DbgPrintEx(DPFLTR_IHVDRIVER_ID, level, format, __VA_ARGS__)

#define LOG_PREFIX                      DRIVER_NAME "!" __FUNCTION__ ": "
#define DsLogInfo(format, ...)          DsDbgPrint(DPFLTR_INFO_LEVEL, LOG_PREFIX format "\n", __VA_ARGS__)
#define DsLogError(format, ...)         DsDbgPrint(DPFLTR_ERROR_LEVEL, LOG_PREFIX format "\n", __VA_ARGS__)
#define DsLogTrace(format, ...)         DsDbgPrint(DPFLTR_TRACE_LEVEL, LOG_PREFIX format "\n", __VA_ARGS__)

#else

#define DsLogInfo   NOP_FUNCTION
#define DsLogError  NOP_FUNCTION
#define DsLogTrace  NOP_FUNCTION

#endif // DBG

/* Macro helpers */
#define __VA_PASS__(x) x
#define __GET_MACRO_2__(_1, _2, name, ...) name

/* Routine definitions */

#ifdef DBG
#define __IRQL_ASSERT(irql) NT_ASSERT(KeGetCurrentIrql() <= irql)
#else
#define __IRQL_ASSERT(irql) NOP_FUNCTION
#endif // DBG

#define DSR_INIT(irql) \
    __IRQL_ASSERT(irql); \
    NTSTATUS status = STATUS_SUCCESS;

#define DSR_CLEANUP() goto cleanup;

#define DSR_CLEANUP_ON_FAIL() \
    if (!NT_SUCCESS(status)) \
        DSR_CLEANUP();

#define __DSR_ASSERT_WITH_SUPPRESS(op, s) \
    status = op; \
    s; \
    DSR_CLEANUP_ON_FAIL();

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

#define DSR_CLEANUP_START() \
    cleanup: \
    if (!NT_SUCCESS(status)) { \
        DsLogError("Unexpected error: 0x%08X", status);

#define DSR_CLEANUP_END()     }

#define DSR_CLEANUP_EMPTY()     DSR_CLEANUP_START() DSR_CLEANUP_END()
#define DSR_STATUS              status
#define DSR_SUCCESS             NT_SUCCESS(status)
