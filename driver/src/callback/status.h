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
#include <driver.h>
#include <status.h>

// Translates pre-op NTSTATUS to FLT_PREOP_CALLBACK_STATUS enum value
#define TO_FLT_PREOP_CALLBACK_STATUS(status)        ((FLT_PREOP_CALLBACK_STATUS)((status) & 0x7))

#ifdef DBG

// Checks whether NTSTATUS relates to FLT_PREOP_CALLBACK_STATUS
#define IS_FLT_PREOP_CALLBACK_STATUS(status)        ((~status & STATUS_PREOP_SUCCESS_WITH_CALLBACK) == 0)

// Asserts NTSTATUS relates to FLT_PREOP_CALLBACK_STATUS
#define ASSERT_FLT_PREOP_CALLBACK_STATUS(status)    \
    if (!IS_FLT_PREOP_CALLBACK_STATUS(status))      \
        DbgRaiseAssertionFailure();

#else

// Does nothing
#define ASSERT_FLT_PREOP_CALLBACK_STATUS(status)    NOP_FUNCTION

#endif
