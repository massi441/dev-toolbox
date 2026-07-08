#pragma once

#include <wil/resource.h>
#include <string>

namespace ml {

wil::unique_handle findProcess(const std::wstring& processName, DWORD access = PROCESS_ALL_ACCESS);
std::wstring toWString(const std::string& str);

}
