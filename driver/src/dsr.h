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
#include "driver.h"

/* Driver simple routines */

#define __VA_PASS__(x) x
#define __GET_MACRO_2__(_1, _2, name, ...) name
#define __GET_MACRO_1__(_1, name, ...) name

#ifdef DBG
#define __IRQL_ASSERT(irql)                         \
    NT_ASSERT(KeGetCurrentIrql() <= irql)
#else
#define __IRQL_ASSERT(irql)                         \
    NOP_FUNCTION
#endif // DBG

#define DSR_SUPPRESS(s)                             \
    if (DSR_STATUS == s)                            \
        DSR_STATUS = STATUS_SUCCESS;

#define DSR_STATUS                                  __status
#define DSR_SUCCESS                                 NT_SUCCESS(DSR_STATUS)

#define DSR_ENTER(irql)                             \
    __IRQL_ASSERT(irql);                            \
    NTSTATUS DSR_STATUS = STATUS_SUCCESS;

#define DSR_LEAVE()                                 \
        goto __exit;

#define DSR_RETURN(status)                          \
    {                                               \
        DSR_STATUS = status;                        \
        goto __exit;                                \
    }

#define DSR_ASSERT_SUCCESS()                        \
    if (!DSR_SUCCESS)                               \
        goto __error_handler;

#define __DSR_ASSERT_WITH_SUPPRESS(status, suppress)\
    DSR_STATUS = status;                            \
    suppress;                                       \
    DSR_ASSERT_SUCCESS();

#define __DSR_ASSERT(status)                        \
    DSR_STATUS = status;                            \
    DSR_ASSERT_SUCCESS();

#define DSR_ASSERT(...)                             \
    __VA_PASS__(__GET_MACRO_2__(                    \
                __VA_ARGS__,                        \
                __DSR_ASSERT_WITH_SUPPRESS,         \
                __DSR_ASSERT                        \
                )(__VA_ARGS__)                      \
    )

#if defined(DBG) && defined(RAISE_ASSERTION_FAILURE)
#define DsRaiseAssertonFailure()                    DbgRaiseAssertionFailure()
#else
#define DsRaiseAssertonFailure()                    NOP_FUNCTION
#endif // RAISE_ASSERTION_FAILURE

#define __DSR_ERROR_STRING                          "Unexpected error: 0x%08X"
#define DSR_ERROR_HANDLER(block)                    \
       goto __exit;                                 \
    __error_handler:                                \
       DsLogError(__DSR_ERROR_STRING, DSR_STATUS);  \
       DsRaiseAssertonFailure();                    \
       block                                        \
    __exit:

#define DSR_DECLARE(var, type)                      \
    type var;                                       \
    RtlZeroMemory(&var, sizeof(type));
