#include "Util.h"

#include <TlHelp32.h>
#include <utf8cpp/utf8.h>

namespace ml {

wil::unique_handle findProcess(const std::wstring& processName, DWORD access) {
    HANDLE snapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapShot == INVALID_HANDLE_VALUE) {
        return wil::unique_handle();
    }

    DWORD pid = -1;
    PROCESSENTRY32W entry{ sizeof(entry) };

    if (Process32FirstW(snapShot, &entry)) {
        do {
            if (processName == entry.szExeFile) {
                pid = entry.th32ProcessID;
            }
        } while (Process32NextW(snapShot, &entry));
    }

    CloseHandle(snapShot);

    if (pid == -1) {
        return wil::unique_handle();
    }

    return wil::unique_handle(OpenProcess(access, FALSE, pid));
}

std::wstring toWString(const std::string& str) {
    std::wstring ws;
    utf8::utf8to16(str.begin(), str.end(), std::back_inserter(ws));
    return ws;
}

}
