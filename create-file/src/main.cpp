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

struct File {
    HANDLE handle;
    wstring name;
    DWORD dwDesiredAccess;
    DWORD dwShareMode;
    DWORD dwCreationDisposition;
    DWORD dwFlagsAndAttributes;

    File(HANDLE handle, wstring &name, DWORD dwDesiredAccess, DWORD dwShareMode, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes) {
        this->handle = handle;
        this->name = name;
        this->dwDesiredAccess = dwDesiredAccess;
        this->dwShareMode = dwShareMode;
        this->dwCreationDisposition = dwCreationDisposition;
        this->dwFlagsAndAttributes = dwFlagsAndAttributes;
    }
};

const wstring S_OPEN(L"open");
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

const wstring S_CREATE_NEW(L"create-new");
const wstring S_CREATE_ALWAYS(L"create-always");
const wstring S_OPEN_EXISTING(L"open-existing");
const wstring S_OPEN_ALWAYS(L"open-always");
const wstring S_TRUNCATE_EXISTING(L"truncate-existing");

const wchar_t F_READ(L'r');
const wchar_t F_WRITE(L'w');
const wchar_t F_DELETE(L'd');

const wchar_t F_DELETE_ON_CLOSE(L'd');
const wchar_t F_BACKUP_SEMANTICS(L'b');
const wchar_t F_POSIX_SEMANTICS(L'p');

vector<File> files;

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

DWORD parseCreationDisposition(wistringstream &input) {
    wstring disposition;
    input >> disposition;
    DWORD dwCreationDisposition = 0;
    if (disposition == S_CREATE_NEW)
        dwCreationDisposition = CREATE_NEW;
    else if (disposition == S_CREATE_ALWAYS)
        dwCreationDisposition = CREATE_ALWAYS;
    else if (disposition == S_OPEN_EXISTING)
        dwCreationDisposition = OPEN_EXISTING;
    else if (disposition == S_OPEN_ALWAYS)
        dwCreationDisposition = OPEN_ALWAYS;
    else if (disposition == S_TRUNCATE_EXISTING)
        dwCreationDisposition = TRUNCATE_EXISTING;
    else
        dwCreationDisposition = OPEN_EXISTING;
    return dwCreationDisposition;
}

wstring getCreationDispositionString(DWORD dwCreationDisposition) {
    switch (dwCreationDisposition) {
        case CREATE_NEW:
            return S_CREATE_NEW;
        case CREATE_ALWAYS:
            return S_CREATE_ALWAYS;
        case OPEN_EXISTING:
            return S_OPEN_EXISTING;
        case OPEN_ALWAYS:
            return S_OPEN_ALWAYS;
        case TRUNCATE_EXISTING:
            return S_TRUNCATE_EXISTING;
    }
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
    DWORD dwDesiredAccess = parseDesiredAccess(input);
    DWORD dwShareMode = parseShareMode(input);
    DWORD dwFlagsAndAttributes = parseFlagsAndAttributes(input);
    DWORD dwCreationDisposition = parseCreationDisposition(input);
    HANDLE hFile = CreateFileW(name.c_str(), dwDesiredAccess, dwShareMode, NULL, dwCreationDisposition, dwFlagsAndAttributes, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        files.push_back(File(hFile, name, dwDesiredAccess, dwShareMode, dwCreationDisposition, dwFlagsAndAttributes));
        wcout << "File " << name << " opened at " << files.size() << "." << endl;
    } else {
        wcout << "Unable to open " << name << ". Error = " << GetLastError() << "." << endl;
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
    wprintf(L"#  |Name            |Access |Share |Flags   |Disposition     |Handle\n");
    for (int i = 0; i < files.size(); ++i) {
        File &file = files[i];
        if (file.handle == INVALID_HANDLE_VALUE)
            continue;
        wprintf(
            L"%-2d |%-15s |%-6s |%-5s |%-7s |%-15s |0x%016llX\n",
            i + 1,
            file.name.c_str(),
            getDesiredAccessString(file.dwDesiredAccess).c_str(),
            getShareModeString(file.dwShareMode).c_str(),
            getFlagsAndAttributesString(file.dwFlagsAndAttributes).c_str(),
            getCreationDispositionString(file.dwCreationDisposition).c_str(),
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

int wmain(int argc, wchar_t *argv[]) {
    while (true) {
        wcout << L"> ";
        wstring line;
        getline(wcin, line);
        wistringstream input(line);
        wstring cmd;
        input >> cmd;
        if (cmd == S_OPEN)
            executeOpen(input);
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
