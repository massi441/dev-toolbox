#include "Util.h"

#include <TlHelp32.h>
#include <utf8cpp/utf8.h>
#include <vector>

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
                break;
            }
        } while (Process32NextW(snapShot, &entry));
    }

    CloseHandle(snapShot);

    if (pid == -1) {
        return wil::unique_handle();
    }

    return wil::unique_handle(OpenProcess(access, FALSE, pid));
}

uint32_t findProcessCount(const std::wstring &processName) {
    HANDLE snapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapShot == INVALID_HANDLE_VALUE) {
        return 0;
    }

    uint32_t count = 0;
    PROCESSENTRY32W entry{ sizeof(entry) };
    if (Process32FirstW(snapShot, &entry)) {
        do {
            if (processName == entry.szExeFile) {
                count++;
            }
        } while (Process32NextW(snapShot, &entry));
    }

    CloseHandle(snapShot);

    return count;
}

std::wstring toWString(const std::string& str) {
    std::wstring ws;
    utf8::utf8to16(str.begin(), str.end(), std::back_inserter(ws));
    return ws;
}

uint32_t toUInt32(const std::string &str, uint32_t fallback) {
    try {
        return std::stoi(str);
    } catch (...) {
        return fallback;
    }
}

bool ensureDirCreated(const std::filesystem::path &path) {
    std::error_code ec;
    if (std::filesystem::exists(path, ec)) {
        return true;
    }

    return std::filesystem::create_directory(path, ec);
}

bool clearDirectory(const std::filesystem::path &path) {
    std::error_code ec;

    std::vector<std::filesystem::path> removeableEntries;
    for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(path, ec)) {
        if (ec) {
            return false;
        }

        removeableEntries.push_back(entry.path());
    }

    for (const std::filesystem::path& removeableEntry : removeableEntries) {
        std::filesystem::remove_all(removeableEntry, ec);
        if (ec) {
            return false;
        }
    }

    return !ec;
}

bool isExistPath(const std::filesystem::path &path) {
    std::error_code ec;
    return std::filesystem::exists(path, ec);
}

}
