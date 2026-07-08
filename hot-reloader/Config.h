#pragma once

#include <string>

namespace ml {

class Config {
public:
    void load();

    const std::string& emuPath() const { return mEmuPath; }
    const std::string& modFolderPath() const { return mModFolderPath; }
    const std::string& sdFolderPath() const { return mSdFolderPath; }

private:
    std::string mEmuPath;
    std::string mModFolderPath;
    std::string mSdFolderPath;
};

}
