#include <iostream>
#include "HotReloader.h"

static ml::HotReloader hotReloader = ml::HotReloader();
static std::atomic<bool> isRunning = false;

BOOL WINAPI ConsoleCloseHandler(DWORD) {
    if (isRunning) {
        hotReloader.restoreFromSd();
        isRunning = false;
    }

    return TRUE;
}

int main() {
    SetConsoleCtrlHandler(ConsoleCloseHandler, TRUE);

    std::string input;
    do {
        std::cout << "Launching emulator at " << hotReloader.getConfig().emuPath() << std::endl;

        if (!hotReloader.tryAttachToProcess()) {
            hotReloader.launchProcess();
        }

        std::cout << "Launched emulator successfully, copying files to sd card" << std::endl;

        hotReloader.copyToSd();

        isRunning = true;

        std::cout << "Copied files to sd card, waiting for emulator to be shutdown" << std::endl;

        hotReloader.waitProcessExit();

        std::cout << "Emulator shutdown successfully, restoring files from sd card" << std::endl;

        hotReloader.restoreFromSd();

        isRunning = false;

        std::cout << "Files successfully restored from sd card, press 1 to restart the emulator, or 0 to exit: ";

        std::cout.flush();

        std::cin >> input;
    } while (input != "0");

    return 0;
}
