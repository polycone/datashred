/*
 * MIT License
 *
 * Copyright (c) 2021 Denis Pakhorukov <xpolycone@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <stdio.h>
#include <Windows.h>
#include <iostream>
#include <vector>
#include <string>
#include <sstream>

using namespace std;

#define FILE_SUPERSEDE                  0x00000000
#define FILE_OPEN                       0x00000001
#define FILE_CREATE                     0x00000002
#define FILE_OPEN_IF                    0x00000003
#define FILE_OVERWRITE                  0x00000004
#define FILE_OVERWRITE_IF               0x00000005

typedef struct _IO_STATUS_BLOCK {
    union {
        NTSTATUS Status;
        PVOID Pointer;
    } DUMMYUNIONNAME;

    ULONG_PTR Information;
} IO_STATUS_BLOCK, *PIO_STATUS_BLOCK;

typedef ULONG ACCESS_MASK;

typedef struct _UNICODE_STRING {
    USHORT Length;
    USHORT MaximumLength;
    PWCH Buffer;
} UNICODE_STRING;
typedef UNICODE_STRING *PUNICODE_STRING;

typedef struct _OBJECT_ATTRIBUTES {
    ULONG Length;
    HANDLE RootDirectory;
    PUNICODE_STRING ObjectName;
    ULONG Attributes;
    PVOID SecurityDescriptor;
    PVOID SecurityQualityOfService;
} OBJECT_ATTRIBUTES;
typedef OBJECT_ATTRIBUTES *POBJECT_ATTRIBUTES;

typedef NTSTATUS(NTAPI *NtCreateFileRoutine)(
    PHANDLE FileHandle,
    ACCESS_MASK DesiredAccess,
    POBJECT_ATTRIBUTES ObjectAttributes,
    PIO_STATUS_BLOCK IoStatusBlock,
    PLARGE_INTEGER AllocationSize,
    ULONG FileAttributes,
    ULONG ShareAccess,
    ULONG CreateDisposition,
    ULONG CreateOptions,
    PVOID EaBuffer,
    ULONG EaLength
);

struct File {
    HANDLE handle;
    wstring name;
    bool nt;
    DWORD dwDesiredAccess;
    DWORD dwShareMode;
    DWORD dwCreationDisposition;
    DWORD dwFlagsAndAttributes;

    File(HANDLE handle, bool nt, wstring &name, DWORD dwDesiredAccess, DWORD dwShareMode, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes) {
        this->handle = handle;
        this->nt = nt;
        this->name = name;
        this->dwDesiredAccess = dwDesiredAccess;
        this->dwShareMode = dwShareMode;
        this->dwCreationDisposition = dwCreationDisposition;
        this->dwFlagsAndAttributes = dwFlagsAndAttributes;
    }
};

const wstring S_OPEN(L"open");
const wstring S_NT_OPEN(L"nt-open");
const wstring S_CLOSE(L"close");
const wstring S_FILE(L"file");
const wstring S_SET(L"set");
const wstring S_LIST(L"list");
const wstring S_EXIT(L"exit");
const wstring S_DELETE(L"delete");
const wstring S_ON_CLOSE(L"on-close");
const wstring S_DELETE_ON_CLOSE(L"delete-on-close");
const wstring S_NONE(L"none");
const wstring S_DISPOSITION(L"disposition");
const wstring S_DISPOSITION_EX(L"disposition-ex");
const wstring S_POSIX_SEMANTICS(L"posix");

const wchar_t F_READ(L'r');
const wchar_t F_WRITE(L'w');
const wchar_t F_DELETE(L'd');

const wchar_t F_DELETE_ON_CLOSE(L'd');
const wchar_t F_BACKUP_SEMANTICS(L'b');
const wchar_t F_POSIX_SEMANTICS(L'p');

HMODULE ntdll;
NtCreateFileRoutine NtCreateFile;
vector<File> files;

vector<wstring> dispositions = {
    wstring(L""),
    wstring(L"create-new"),
    wstring(L"create-always"),
    wstring(L"open-existing"),
    wstring(L"open-always"),
    wstring(L"truncate-existing")
};
vector<wstring> ntDispositions = {
    wstring(L"supersede"),
    wstring(L"open"),
    wstring(L"create"),
    wstring(L"open-if"),
    wstring(L"overwrite"),
    wstring(L"overwrite-if")
};

DWORD parseDesiredAccess(wistringstream &input) {
    wstring access;
    input >> access;
    DWORD dwDesiredAccess = 0;
    if (access.find(F_READ) != -1)
        dwDesiredAccess |= GENERIC_READ;
    if (access.find(F_WRITE) != -1)
        dwDesiredAccess |= GENERIC_WRITE;
    if (access.find(F_DELETE) != -1)
        dwDesiredAccess |= DELETE;
    return dwDesiredAccess;
}

DWORD parseShareMode(wistringstream &input) {
    wstring mode;
    input >> mode;
    DWORD dwShareMode = 0;
    if (mode.find(F_READ) != -1)
        dwShareMode |= FILE_SHARE_READ;
    if (mode.find(F_WRITE) != -1)
        dwShareMode |= FILE_SHARE_WRITE;
    if (mode.find(F_DELETE) != -1)
        dwShareMode |= FILE_SHARE_DELETE;
    return dwShareMode;
}

DWORD parseFlagsAndAttributes(wistringstream &input) {
    wstring flags;
    input >> flags;
    DWORD dwFlagsAndAttributes = 0;
    if (flags.find(F_DELETE_ON_CLOSE) != -1)
        dwFlagsAndAttributes |= FILE_FLAG_DELETE_ON_CLOSE;
    if (flags.find(F_BACKUP_SEMANTICS) != -1)
        dwFlagsAndAttributes |= FILE_FLAG_BACKUP_SEMANTICS;
    if (flags.find(F_POSIX_SEMANTICS) != -1)
        dwFlagsAndAttributes |= FILE_FLAG_POSIX_SEMANTICS;
    return dwFlagsAndAttributes;
}

DWORD parseCreationDisposition(wistringstream &input, vector<wstring> const &dispositions, DWORD defaultDisposition) {
    wstring disposition;
    input >> disposition;
    DWORD dwCreationDisposition = 0;
    for (int i = 0; i < dispositions.size(); i++)
        if (!dispositions[i].empty() && disposition == dispositions[i])
            return i;
    return defaultDisposition;
}

wstring getCreationDispositionString(DWORD dwCreationDisposition, vector<wstring> const &dispositions) {
    return dispositions[dwCreationDisposition];
}

wstring getDesiredAccessString(DWORD dwDesiredAccess) {
    wstring desiredAccess;
    if (dwDesiredAccess & GENERIC_READ)
        desiredAccess += F_READ;
    if (dwDesiredAccess & GENERIC_WRITE)
        desiredAccess += F_WRITE;
    if (dwDesiredAccess & DELETE)
        desiredAccess += F_DELETE;
    return desiredAccess;
}

wstring getShareModeString(DWORD dwShareMode) {
    wstring shareMode;
    if (dwShareMode & FILE_SHARE_READ)
        shareMode += F_READ;
    if (dwShareMode & FILE_SHARE_WRITE)
        shareMode += F_WRITE;
    if (dwShareMode & FILE_SHARE_DELETE)
        shareMode += F_DELETE;
    return shareMode;
}

wstring getFlagsAndAttributesString(DWORD dwFlagsAndAttributes) {
    wstring shareMode;
    if (dwFlagsAndAttributes & FILE_FLAG_DELETE_ON_CLOSE)
        shareMode += F_DELETE_ON_CLOSE;
    if (dwFlagsAndAttributes & FILE_FLAG_BACKUP_SEMANTICS)
        shareMode += F_BACKUP_SEMANTICS;
    return shareMode;
}

void executeOpen(wistringstream &input) {
    wstring name;
    input >> name;
    if (name.empty()) {
        wcout << "File name isn't specified" << endl;
        return;
    }
    DWORD dwDesiredAccess = parseDesiredAccess(input);
    DWORD dwShareMode = parseShareMode(input);
    DWORD dwFlagsAndAttributes = parseFlagsAndAttributes(input);
    DWORD dwCreationDisposition = parseCreationDisposition(input, dispositions, OPEN_EXISTING);
    HANDLE hFile = CreateFileW(name.c_str(), dwDesiredAccess, dwShareMode, NULL, dwCreationDisposition, dwFlagsAndAttributes, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        files.push_back(File(hFile, false, name, dwDesiredAccess, dwShareMode, dwCreationDisposition, dwFlagsAndAttributes));
        wcout << "File " << name << " opened at " << files.size() << "." << endl;
    } else {
        wcout << "Unable to open " << name << ". Error = " << GetLastError() << "." << endl;
    }
}

void executeNtOpen(wistringstream &input) {
    wstring name;
    input >> name;
    DWORD dwDesiredAccess = parseDesiredAccess(input);
    if (dwDesiredAccess == 0)
        dwDesiredAccess = SYNCHRONIZE | FILE_READ_ATTRIBUTES;
    DWORD dwShareMode = parseShareMode(input);
    DWORD dwCreationDisposition = parseCreationDisposition(input, ntDispositions, FILE_OPEN);
    if (name.empty()) {
        wcout << "File name isn't specified" << endl;
        return;
    }
    wstring fqn;
    if (name[0] != L'\\') {
        DWORD length = GetFullPathNameW(name.c_str(), 0, NULL, NULL);
        PWCH nameBuffer = (PWCH)malloc(length * sizeof(WCHAR));
        GetFullPathNameW(name.c_str(), length, nameBuffer, NULL);
        fqn = wstring(L"\\??\\");
        fqn.append(nameBuffer, length - 1);
        free(nameBuffer);
    } else {
        fqn = name;
    }

    UNICODE_STRING uFqn;
    uFqn.Length = fqn.length() * sizeof(WCHAR);
    uFqn.MaximumLength = fqn.length() * sizeof(WCHAR);
    uFqn.Buffer = (PWCH)fqn.c_str();

    HANDLE hFile = INVALID_HANDLE_VALUE;
    IO_STATUS_BLOCK status;
    OBJECT_ATTRIBUTES attibutes;
    attibutes.Length = sizeof(OBJECT_ATTRIBUTES);
    attibutes.RootDirectory = NULL;
    attibutes.ObjectName = &uFqn;
    attibutes.Attributes = 0;
    attibutes.SecurityDescriptor = NULL;
    attibutes.SecurityQualityOfService = NULL;

    NTSTATUS result = NtCreateFile(&hFile, dwDesiredAccess, &attibutes, &status, NULL, 0, dwShareMode, dwCreationDisposition, 0, NULL, 0);
    if (hFile != INVALID_HANDLE_VALUE) {
        files.push_back(File(hFile, true, name, dwDesiredAccess, dwShareMode, dwCreationDisposition, 0));
        wcout << "File " << name << " opened at " << files.size() << "." << endl;
    } else {
        wcout << "Unable to open " << name << ". Error = 0x" << hex << result << "." << endl;
    }
}

void executeClose(wistringstream &input) {
    input >> ws;
    while (!input.eof()) {
        size_t fileIndex;
        input >> fileIndex;
        if (fileIndex < 1 || fileIndex > files.size()) {
            wcout << "Invalid index " << fileIndex << "." << endl;
            continue;
        }
        File &file = files[fileIndex - 1];
        if (file.handle == INVALID_HANDLE_VALUE) {
            wcout << "File " << file.name << " at " << fileIndex << " already closed." << endl;
        } else {
            CloseHandle(file.handle);
            file.handle = INVALID_HANDLE_VALUE;
            wcout << "File " << file.name << " at " << fileIndex << " closed." << endl;
        }
    }
}

void executeList() {
    wprintf(L"#  |Name            |Access |Share |Flags   |Disposition     |NT |Handle\n");
    for (int i = 0; i < files.size(); ++i) {
        File &file = files[i];
        if (file.handle == INVALID_HANDLE_VALUE)
            continue;
        wprintf(
            L"%-2d |%-15s |%-6s |%-5s |%-7s |%-15s |%-2s |0x%016llX\n",
            i + 1,
            file.name.c_str(),
            getDesiredAccessString(file.dwDesiredAccess).c_str(),
            getShareModeString(file.dwShareMode).c_str(),
            getFlagsAndAttributesString(file.dwFlagsAndAttributes).c_str(),
            getCreationDispositionString(file.dwCreationDisposition, file.nt ? ntDispositions : dispositions).c_str(),
            file.nt ? L"*" : L"",
            (unsigned __int64)file.handle
        );
    }
}

void executeFileSetDisposition(size_t fileIndex, wistringstream &input) {
    wstring disposition;
    input >> disposition;
    FILE_DISPOSITION_INFO info;
    if (disposition == S_DELETE) {
        info.DeleteFileW = TRUE;
    } else if (disposition == S_NONE) {
        info.DeleteFileW = FALSE;
    } else {
        wcout << "Unknown disposition " << disposition << endl;
        return;
    }
    File &file = files[fileIndex - 1];
    bool result = SetFileInformationByHandle(file.handle, FileDispositionInfo, &info, sizeof(FILE_DISPOSITION_INFO));
    if (result)
        wcout << "Disposition set for file " << file.name << " at " << fileIndex << "." << endl;
    else
        wcout << "Unable to set disposition for file " << file.name << " at " << fileIndex << ". Error = " << GetLastError() << "." << endl;
}

void executeFileSetDispositionEx(size_t fileIndex, wistringstream &input) {
    wstring option;
    FILE_DISPOSITION_INFO_EX info;
    info.Flags = 0;
    input >> ws;
    while (!input.eof()) {
        input >> option;
        if (option == S_DELETE) {
            info.Flags |= FILE_DISPOSITION_FLAG_DELETE;
        } else if (option == S_POSIX_SEMANTICS) {
            info.Flags |= FILE_DISPOSITION_FLAG_POSIX_SEMANTICS;
        } else if (option == S_ON_CLOSE) {
            info.Flags |= FILE_DISPOSITION_FLAG_ON_CLOSE;
        } else {
            wcout << "Unknown disposition option " << option << endl;
            return;
        }
    }
    File &file = files[fileIndex - 1];
    bool result = SetFileInformationByHandle(file.handle, FileDispositionInfoEx, &info, sizeof(FILE_DISPOSITION_INFO_EX));
    if (result) {
        wcout << "Extended disposition set for file " << file.name << " at " << fileIndex << "." << endl;
    } else {
        wcout << "Unable to set extended disposition for file " << file.name << " at " << fileIndex << ". Error = " << GetLastError() << "." << endl;
    }
}

void executeFileSet(size_t fileIndex, wistringstream &input) {
    wstring type;
    input >> type;
    if (type == S_DISPOSITION)
        executeFileSetDisposition(fileIndex, input);
    else if (type == S_DISPOSITION_EX)
        executeFileSetDispositionEx(fileIndex, input);
    else
        wcout << "Unknown type " << type << endl;
}

void executeFile(wistringstream &input) {
    size_t fileIndex;
    wstring operation;
    input >> fileIndex >> operation;
    if (fileIndex < 1 || fileIndex > files.size()) {
        wcout << "Invalid index " << fileIndex << "." << endl;
        return;
    }
    if (operation == S_SET)
        executeFileSet(fileIndex, input);
    else
        wcout << "Unknown operation " << operation << endl;
}

void initNt() {
    ntdll = LoadLibraryW(L"ntdll.dll");
    if (ntdll == NULL) {
        wcout << "Warning: Unable to load ntdll.dll. Error = " << GetLastError() << "." << endl;
        return;
    }
    NtCreateFile = (NtCreateFileRoutine)GetProcAddress(ntdll, "NtCreateFile");
    if (NtCreateFile == NULL)
        wcout << "Warning: Unable to locate procedure NtCreateFile. Error = " << GetLastError() << "." << endl;
}

int wmain(int argc, wchar_t *argv[]) {
    initNt();
    while (true) {
        wcout << L"> ";
        wstring line;
        getline(wcin, line);
        wistringstream input(line);
        wstring cmd;
        input >> cmd;
        if (cmd == S_OPEN)
            executeOpen(input);
        else if (cmd == S_NT_OPEN)
            executeNtOpen(input);
        else if (cmd == S_CLOSE)
            executeClose(input);
        else if (cmd == S_FILE)
            executeFile(input);
        else if (cmd == S_LIST)
            executeList();
        else if (cmd == S_EXIT)
            break;
        else
            wcout << "Unknown command " << cmd << endl;
    }
    return 0;
}
