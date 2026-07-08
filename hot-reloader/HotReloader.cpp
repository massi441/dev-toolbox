#include "HotReloader.h"

#include <filesystem>
#include <iostream>

namespace ml {

static constexpr std::filesystem::copy_options sCopyOptions =
    std::filesystem::copy_options::recursive |
    std::filesystem::copy_options::overwrite_existing;

HotReloader::HotReloader() {
    mConfig = std::make_unique<Config>();
    mConfig->load();
}

void HotReloader::launchProcess() {
    STARTUPINFOA startup_info = { .cb = sizeof(STARTUPINFOA) };

    CreateProcessA(
        mConfig->emuPath().c_str(),
        nullptr,
        nullptr,
        nullptr,
        FALSE,
        CREATE_NEW_CONSOLE,
        nullptr,
        nullptr,
        &startup_info,
        &mProcessInfo
    );
}

bool HotReloader::tryAttachToProcess() {
    return false;
}

void HotReloader::waitProcessExit() {
    WaitForSingleObject(mProcessInfo.hProcess, INFINITE);

    mProcessInfo.reset();
}

void HotReloader::copyToSd() const {
    std::error_code ec;
    std::filesystem::remove_all(mConfig->sdPath(), ec);
    std::filesystem::copy(mConfig->modPath(), mConfig->sdPath(), sCopyOptions, ec);

    if (ec) {
        // std::cerr << "Failed to copy \"" << srcPath << "\" to \"" << destPath << "\": " << ec.message() << std::endl;
    }
}

void HotReloader::restoreFromSd() const {
    std::error_code ec;
    std::filesystem::remove_all(mConfig->modPath(), ec);
    std::filesystem::copy(mConfig->sdPath(), mConfig->modPath(), sCopyOptions, ec);

    if (ec) {
        // std::cerr << "Failed to copy \"" << srcPath << "\" to \"" << destPath << "\": " << ec.message() << std::endl;
    }
}

}
