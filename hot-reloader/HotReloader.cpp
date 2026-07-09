#include "HotReloader.h"

#include <filesystem>
#include "Util.h"

namespace ml {

static constexpr std::filesystem::copy_options sCopyOptions =
    std::filesystem::copy_options::recursive |
    std::filesystem::copy_options::overwrite_existing;

static constexpr const char* sBackupsFolderName = "backups";

HotReloader::HotReloader() {
    mConfig = std::make_unique<Config>();
    mConfig->load();
    mShutdownEvent = wil::unique_event(wil::EventOptions::ManualReset);
}

bool HotReloader::tryAttachToEmu() {
    std::filesystem::path path = mConfig->emuPath();
    std::wstring emuW = ml::toWString(path.filename().string());

    mAttachedEmuHandle = ml::findProcess(emuW);

    return mAttachedEmuHandle.is_valid();
}

bool HotReloader::launchEmu() {
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

bool HotReloader::waitEmuExit() {
    HANDLE waitHandles[] = { getCurrentHandle(), mShutdownEvent.get() };
    DWORD waitObject = WaitForMultipleObjects(std::size(waitHandles), waitHandles, false, INFINITE );

    if (waitObject == WAIT_FAILED) {
        return false;
    }

    // if the wait was complete because of a shutdown
    if (waitObject == WAIT_OBJECT_0 + 1)  {
        if (!this->closeEmu()) {
            return false;
        }

        if (WaitForSingleObject(this->getCurrentHandle(), INFINITE) == WAIT_FAILED) {
            return false;
        }
    }

    mEmuProcess.reset();
    mAttachedEmuHandle.reset();
    mShutdownEvent.ResetEvent();

    return true;
}

bool HotReloader::backupMod() const {
    std::filesystem::path backupsPath = mConfig->modPath();
    backupsPath.replace_filename(sBackupsFolderName);

    std::error_code ec;
    if (ml::isExistPath(backupsPath)) {
        // remove oldest back up (if present)
        std::filesystem::path maxBackupPath = backupsPath / std::to_string(mConfig->backupDepth());

        if (ml::isExistPath(maxBackupPath)) {
            std::filesystem::remove_all(maxBackupPath, ec);

            if (ec) {
                return false;
            }
        }

        // shift all back up folder numbers up by one (starting at the second to last backup)
        for (uint32_t i = mConfig->backupDepth() - 1; i > 0; i--) {
            std::filesystem::path backupPath = backupsPath / std::to_string(i);
            std::filesystem::path newBackupPath = backupsPath / std::to_string(i + 1);

            if (!ml::isExistPath(backupPath)) {
                continue;
            }

            std::filesystem::rename(backupPath, newBackupPath, ec);
        }
    } else if (!std::filesystem::create_directory(backupsPath, ec)) {
        return false;
    }

    std::filesystem::path newestBackupPath = backupsPath / std::to_string(1);
    std::filesystem::copy(mConfig->modPath(), newestBackupPath, sCopyOptions, ec);

    return !ec;
}

bool HotReloader::removeLastBackup() const {

}

bool HotReloader::shiftBackups() const {

}

bool HotReloader::copyModFromSd() const {
    if (!ml::clearDirectory(mConfig->modPath())) {
        return false;
    }

    std::error_code ec;
    std::filesystem::copy(mConfig->sdPath(), mConfig->modPath(), sCopyOptions, ec);

    return !ec;
}

void HotReloader::requestEmuShutdown() const {
    mShutdownEvent.SetEvent();
}

bool HotReloader::closeEmu() const {
    return TerminateProcess(this->getCurrentHandle(), 1);
}

HANDLE HotReloader::getCurrentHandle() const {
    return mAttachedEmuHandle.is_valid()
       ? mAttachedEmuHandle.get()
       : mEmuProcess.hProcess;
}

}
