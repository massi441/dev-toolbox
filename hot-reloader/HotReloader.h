#pragma once

#include <memory>
#include <wil/resource.h>
#include "Config.h"

namespace ml {

class HotReloader {
public:
    HotReloader();

    bool tryAttachToEmu();
    bool launchEmu();
    bool waitEmuExit();
    bool copyModToSd() const;
    bool restoreModFromSd() const;

    const Config& getConfig() const { return *mConfig; }

private:
    std::unique_ptr<Config> mConfig;
    wil::unique_handle mRunningEmuHandle; // emu handle if it's already running
    wil::unique_process_information mEmuProcess; // emu process info if created by the hot reloader
};

}
