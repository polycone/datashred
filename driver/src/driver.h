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
#include <fltKernel.h>
#include <dontuse.h>
#include "status.h"

#pragma prefast(disable:__WARNING_ENCODE_MEMBER_FUNCTION_POINTER, "Not valid for kernel mode drivers")

#ifdef DBG
#pragma warning(disable:4100) // Unreferenced formal parameter
#pragma warning(disable:4102) // Unreferenced label
#pragma warning(disable:4189) // Unreferenced local variable
#pragma warning(disable:4702) // Unreacheable code
#endif

#pragma warning(disable:4214) // Bit field types other than int
#pragma warning(disable:4201) // Nameless structs/unions

/* Empty aliases */
#define NO_FLAGS                                    0
#define NO_CALLBACK                                 NULL
#define NO_CONTEXT                                  NULL

/* Configuration */
#define DS_DEFAULT_POOL_TAG                         'pDsD'

/* Kernel debug messaging */
#ifdef DBG
#define DsDbgPrint(level, format, ...)              DbgPrintEx(DPFLTR_IHVDRIVER_ID, level, format, __VA_ARGS__)
#define LOG_PREFIX                                  DRIVER_NAME "!" __FUNCTION__ ": "
#define DsLogInfo(format, ...)                      DsDbgPrint(DPFLTR_INFO_LEVEL, LOG_PREFIX format "\n", __VA_ARGS__)
#define DsLogError(format, ...)                     DsDbgPrint(DPFLTR_ERROR_LEVEL, LOG_PREFIX format "\n", __VA_ARGS__)
#define DsLogTrace(format, ...)                     DsDbgPrint(DPFLTR_TRACE_LEVEL, LOG_PREFIX format "\n", __VA_ARGS__)
#else
#define DsLogInfo   NOP_FUNCTION
#define DsLogError  NOP_FUNCTION
#define DsLogTrace  NOP_FUNCTION
#endif // DBG

#define FltReleaseContextSafe(ctx)                  \
    if (ctx != NO_CONTEXT)                          \
        FltReleaseContext(ctx);

#define FltObjectDereferenceSafe(obj)               \
    if (obj != NULL)                                \
        FltObjectDereference(obj);                  \

/* Filter functions shortcuts */
#define DsQueryStandardInformationFile(objs, info)  \
    FltQueryInformationFile(                        \
        objs->Instance,                             \
        objs->FileObject,                           \
        info,                                       \
        sizeof(FILE_STANDARD_INFORMATION),          \
        FileStandardInformation,                    \
        NULL                                        \
    )
