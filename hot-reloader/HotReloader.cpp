#include "HotReloader.h"

#include <filesystem>
#include "Util.h"

namespace ml {

static constexpr std::filesystem::copy_options sCopyOptions =
    std::filesystem::copy_options::recursive |
    std::filesystem::copy_options::overwrite_existing;

HotReloader::HotReloader() {
    mConfig = std::make_unique<Config>();
    mConfig->load();
}

bool HotReloader::tryAttachToProcess() {
    std::filesystem::path path = mConfig->emuPath();
    std::wstring emuW = toWString(path.filename().string());

    mRunningEmuHandle = findProcess(emuW);

    return mRunningEmuHandle.is_valid();
}

bool HotReloader::launchProcess() {
    STARTUPINFOA startup_info = { .cb = sizeof(STARTUPINFOA) };

    return CreateProcessA(
        mConfig->emuPath().c_str(),
        nullptr,
        nullptr,
        nullptr,
        FALSE,
        CREATE_NEW_CONSOLE,
        nullptr,
        nullptr,
        &startup_info,
        &mEmuProcess
    );
}

bool HotReloader::waitProcessExit() {
    HANDLE waitHandle = mRunningEmuHandle.is_valid()
        ? mRunningEmuHandle.get()
        : mEmuProcess.hProcess;

    WaitForSingleObject(waitHandle, INFINITE);

    mEmuProcess.reset();
    mRunningEmuHandle.reset();

    return true;
}

bool HotReloader::copyToSd() const {
    std::error_code ec;
    std::filesystem::remove_all(mConfig->sdPath(), ec);
    std::filesystem::copy(mConfig->modPath(), mConfig->sdPath(), sCopyOptions, ec);

    if (ec) {
        return false;
    }

    return true;
}

bool HotReloader::restoreFromSd() const {
    std::error_code ec;
    std::filesystem::remove_all(mConfig->modPath(), ec);
    std::filesystem::copy(mConfig->sdPath(), mConfig->modPath(), sCopyOptions, ec);

    if (ec) {
        return false;
    }

    return true;
}

}
