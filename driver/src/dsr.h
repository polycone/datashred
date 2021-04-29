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
#include <wdm.h>

/* Driver simple routines */

#ifdef DBG
#define __IRQL_ASSERT(irql)                         \
    NT_ASSERT(KeGetCurrentIrql() <= irql)
#else
#define __IRQL_ASSERT(irql)                         \
    NOP_FUNCTION
#endif // DBG

#define DSR_INIT(irql)                              \
    __IRQL_ASSERT(irql);                            \
    NTSTATUS status = STATUS_SUCCESS;

#define __VA_PASS__(x) x
#define __GET_MACRO_2__(_1, _2, name, ...) name

#define DSR_GOTO_CLEANUP() goto cleanup;
#define DSR_RESET() status = STATUS_SUCCESS;

#define DSR_CLEANUP_ON_FAIL()                       \
    if (!NT_SUCCESS(status))                        \
        DSR_GOTO_CLEANUP();

#define __DSR_ASSERT_WITH_SUPPRESS(op, s)           \
    status = op;                                    \
    s;                                              \
    DSR_CLEANUP_ON_FAIL();

#define __DSR_ASSERT(op)                            \
    __DSR_ASSERT_WITH_SUPPRESS(op, )

#define DSR_ASSERT(...)                             \
    __VA_PASS__(__GET_MACRO_2__(                    \
                __VA_ARGS__,                        \
                __DSR_ASSERT_WITH_SUPPRESS,         \
                __DSR_ASSERT                        \
                )(__VA_ARGS__)                      \
    )

#define DSR_SUPPRESS(s)                             \
    if (status == s)                                \
        status = STATUS_SUCCESS;

#define DSR_CLEANUP                                 \
cleanup:                                            \
    if (!NT_SUCCESS(status))

#define DSR_STATUS                                  status
#define DSR_SUCCESS                                 NT_SUCCESS(status)

#define DSR_DECLARE(var, type)                      \
    type var;                                       \
    RtlZeroMemory(&var, sizeof(type));
