#pragma once

#include <memory>
#include <wil/resource.h>
#include "Config.h"

namespace ml {

class HotReloader {
public:
    HotReloader();

    void launchProcess();
    bool tryAttachToProcess();
    void waitProcessExit();
    void copyToSd() const;
    void restoreFromSd() const;

    const Config& getConfig() const { return *mConfig; }

private:
    std::unique_ptr<Config> mConfig;
    wil::unique_process_information mProcessInfo;
};

}
