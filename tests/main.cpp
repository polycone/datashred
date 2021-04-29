#include <stdio.h>
#include <Windows.h>

bool flagPresent(wchar_t *flags, wchar_t flag) {
    return wcschr(flags, flag);
}

int wmain(int argc, wchar_t *argv[]) {
    wchar_t *flags = argv[2];
    DWORD dwFlagsAndAttributes = 0;
    if (flagPresent(flags, L'd')) {
        dwFlagsAndAttributes |= FILE_FLAG_DELETE_ON_CLOSE;
    }
    if (flagPresent(flags, L'b')) {
        dwFlagsAndAttributes |= FILE_FLAG_BACKUP_SEMANTICS;
    }
    HANDLE foo = CreateFileW(argv[1], GENERIC_READ | GENERIC_WRITE | DELETE, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL, OPEN_EXISTING, dwFlagsAndAttributes, NULL);
    printf("HANDLE = 0x%p\n", foo);
    system("pause");
    return 0;
}
