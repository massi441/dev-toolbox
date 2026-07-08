#include <iostream>
#include "HotReloader.h"

static ml::HotReloader hotReloader = ml::HotReloader();
static std::atomic<bool> isRunning = false;

BOOL WINAPI ConsoleCloseHandler(DWORD) {
    if (isRunning) {
        std::cout << "Intercepted shutdown, restoring files from sd folder" << std::endl;
        if (hotReloader.restoreModFromSd()) {
            std::cout << "Successfully restored from sd folder, shutting down..." << std::endl;
        } else {
            std::cout << "Failed to restore from sd folder" << std::endl;
        }

        isRunning = false;
    }

    return FALSE;
}

int main() {
    SetConsoleCtrlHandler(ConsoleCloseHandler, TRUE);

    std::cout << "Launching hot reloader for: " << hotReloader.getConfig().emuPath() << std::endl;

    if (!hotReloader.tryAttachToEmu() && !hotReloader.launchEmu()) {
        std::cout << "Launched emulator successfully" << std::endl;
        return 1;
    }

    std::cout << "Hot reloader initialized, copying files to sd card..." << std::endl;

    if (!hotReloader.copyModToSd()) {
        std::cerr << "Failed to copy to sd folder" << std::endl;
        return 1;
    }

    isRunning = true;

    std::cout << "Copied files to sd card, waiting for emulator to be shutdown" << std::endl;

    if (!hotReloader.waitEmuExit()) {
        std::cout << "Failed to wait for emulator shutdown" << std::endl;
        return 1;
    }

    std::cout << "Emulator shutdown successfully, restoring files from sd card" << std::endl;

    if (!hotReloader.restoreModFromSd()) {
        std::cerr << "Failed to restore from sd folder" << std::endl;
        return 1;
    }

    isRunning = false;

    std::string line;
    std::getline(std::cin, line);

    return 0;
}
