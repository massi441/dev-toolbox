#include "HotReloader.h"

#include <iostream>
#include <semaphore>

#include "Util.h"

static ml::HotReloader hotReloader = ml::HotReloader();
static std::binary_semaphore closeSem = std::binary_semaphore(0);

BOOL WINAPI ConsoleCloseHandler(DWORD) {
    std::cout << "Intercepted shutdown, emulator will be closed..." << std::endl;

    hotReloader.requestEmuShutdown();
    closeSem.acquire();
    closeSem.release();

    return FALSE;
}

int main() {
    if (ml::findProcessCount(ml::toWString(APP_NAME)) > 1) {
        std::cout << "Hot reloader already opened, closing this one..." << std::endl;
        return 1;
    }

    ShowWindow(GetConsoleWindow(), SW_SHOWMINNOACTIVE);

    if (!SetConsoleCtrlHandler(ConsoleCloseHandler, TRUE)) {
        std::cout << "Failed to set console handler" << std::endl;
        return 1;
    }

    const ml::Config& config = hotReloader.getConfig();
    if (!config.isValid()) {
        std::cerr << "Failed to initialize config, one of the following paths was not found: \n" << config.toString() << std::endl;
        return 1;
    }

    std::cout << "Launching hot reloader for: " << config.emuPath() << std::endl;

    if (hotReloader.tryAttachToEmu()) {
        std::cout << "Successfully attached to emulator" << std::endl;
    } else if (hotReloader.launchEmu()) {
        std::cout << "Successfully launcher emulator" << std::endl;
    } else {
        std::cerr << "Failed to attach/launch emulator" << std::endl;
        return 1;
    }

    std::cout << "Backing up mod and copying files from sd card..." << std::endl;

    if (!hotReloader.backupMod()) {
        std::cout << "Failed to backup mod from sd folder" << std::endl;
        return 1;
    }

    if (!hotReloader.copyModFromSd()) {
        std::cerr << "Failed to copy files from sd folder" << std::endl;
        return 1;
    }

    std::cout << "Copied files from sd card, waiting for emulator to be shutdown..." << std::endl;

    if (!hotReloader.waitEmuExit()) {
        std::cout << "Failed to wait for emulator shutdown" << std::endl;
        return 1;
    }

    std::cout << "Emulator shutdown successfully, restoring files from sd card" << std::endl;

    if (!hotReloader.copyModFromSd()) {
        closeSem.release(); // avoids a deadlock in case an error happens
        std::cerr << "Failed to restore from sd folder" << std::endl;
        return 1;
    }

    closeSem.release();

    std::cout << "Mod successfully restored from sd card, hot reloader is closing..." << std::endl;

    return 0;
}
