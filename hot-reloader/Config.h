#pragma once

#include <string>
#include <vector>

namespace ml {

class Config {
public:
    void load();

    const std::string& emuPath() const { return mEmuPath; }
    const std::string& modPath() const { return mModPath; }
    const std::string& sdPath() const { return mSdPath; }
    const std::vector<std::string>& folders() const { return mFolders; }

private:
    std::string mEmuPath;
    std::string mModPath;
    std::string mSdPath;
    std::vector<std::string> mFolders;
};

}
