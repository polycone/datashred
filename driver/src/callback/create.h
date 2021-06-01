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

typedef NTSTATUS(FLTAPI *FLT_GET_CONTEXT_ROUTINE)(
    _In_ PFLT_INSTANCE Instance,
    _In_ PFILE_OBJECT FileObject,
    _Outptr_ PFLT_CONTEXT *Context
);

typedef NTSTATUS(FLTAPI *FLT_SET_CONTEXT_ROUTINE)(
    _In_ PFLT_INSTANCE Instance,
    _In_ PFILE_OBJECT FileObject,
    _In_ FLT_SET_CONTEXT_OPERATION Operation,
    _In_ PFLT_CONTEXT NewContext,
    _Outptr_opt_result_maybenull_ PFLT_CONTEXT *OldContext
);

typedef NTSTATUS(*PDS_CREATE_CONTEXT_ROUTINE)(_In_opt_ PVOID Parameters, _Out_ PFLT_CONTEXT Context);

typedef struct _DS_CONTEXT_DESCRIPTOR {
    FLT_GET_CONTEXT_ROUTINE CONST Get;
    FLT_SET_CONTEXT_ROUTINE CONST Set;
    PDS_CREATE_CONTEXT_ROUTINE CONST Create;
} DS_CONTEXT_DESCRIPTOR, *PDS_CONTEXT_DESCRIPTOR;
typedef CONST struct _DS_CONTEXT_DESCRIPTOR *PCDS_CONTEXT_DESCRIPTOR;

#ifdef DBG
#define STREAM_CONTEXT_PARAMETERS_MAGIC             0x574EA770
#define FILE_CONTEXT_PARAMETERS_MAGIC               0x85318FA1
#endif

typedef struct _DS_STREAM_CONTEXT_PARAMETERS {
#ifdef DBG
    ULONG Magic;
#endif
    PFLT_FILTER Filter;
    PDS_INSTANCE_CONTEXT InstanceContext;
    PDS_FILE_CONTEXT FileContext;
    PFLT_FILE_NAME_INFORMATION FileNameInfo;
} DS_STREAM_CONTEXT_PARAMETERS, *PDS_STREAM_CONTEXT_PARAMETERS;

typedef struct _DS_FILE_CONTEXT_PARAMETERS {
#ifdef DBG
    ULONG Magic;
#endif
    PFLT_FILTER Filter;
} DS_FILE_CONTEXT_PARAMETERS, *PDS_FILE_CONTEXT_PARAMETERS;
