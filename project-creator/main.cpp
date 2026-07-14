#include <filesystem>
#include <iostream>

#include "Core/IO/FsUtil.h"
#include "Core/String/StringUtil.h"
#include "Core/Util/ConfigUtil.h"
#include "Core/Util/ConvertUtil.h"
#include "Core/Util/MathHelpers.h"

#include "ResourceType.h"

using namespace ml;
namespace fs = std::filesystem;

// decls
class Config {
public:
    bool load(const char* fileName = "config.txt");

    const std::string& outputFolder() const { return mOutputFolder; }

private:
    std::string mOutputFolder;
};

std::string getProjectName();
int64_t getInputNumber(int64_t lowerBoundIncl, int64_t upperBoundIncl);

bool confirmProjectCreation(const fs::path& destPath);
bool selectTemplate(fs::path* outTemplatePath);
bool createProject(const fs::path& destPath, const fs::path& templatePath);

int main() {
    Config config;

    if (!config.load()) {
        std::cout << "Failed to load config, output folder not found: " << config.outputFolder() << std::endl;
        return 1;
    }

    std::cout << "---- Project Creator ----" << std::endl;
    std::string projectName = getProjectName();

    fs::path destPath = ml::makePath(config.outputFolder(), projectName);
    if (!confirmProjectCreation(destPath)) {
        std::cout << "Project creation aborted" << std::endl;
        return 1;
    }

    fs::path projectTemplatePath;
    if (!selectTemplate(&projectTemplatePath)) {
        return 1;
    }

    std::cout << "Creating \"" << projectName << "\" project from " << projectTemplatePath.filename() << " template..." << std::endl;

    if (!createProject(destPath, projectTemplatePath)) {
        ml::removeDirectory(destPath);
        std::cout << "Nothing was created, verify that the template has valid data" << std::endl;
        return 1;
    }

    std::cout << "Successfully crated project, press any key to exit..." << std::endl;

    getchar();

    return 0;
}

// resource handling

using HandlerFunc = bool (*)(const fs::path& projectFolderPath, const std::string& resourceName, const std::string& templateResourceOutput);

bool handleDirectory(const fs::path& projectFolderPath, const std::string& resourceName, const std::string&) {
    fs::path createPath = ml::makePath(projectFolderPath.c_str(), resourceName);

    std::cout << "Creating directory: " << resourceName << std::endl;

    std::error_code ec;
    fs::create_directory(ml::makePath(projectFolderPath.string(), resourceName), ec);

    if (ec) {
        std::cout << "Failed to create directory: " << ec.message() << std::endl;
    }

    return true;
}

// used for template resources, as they are all copy operations
bool handleTemplateResource(const fs::path& projectFolderPath, const std::string& resourceName, const std::string& templateResourceOutput) {
    fs::path templateResourcePath = ml::fromCurrentPath("resources", resourceName);

    if (!ml::isExistPath(templateResourcePath)) {
        std::cout << "WARNING: Template resource not found: " << resourceName << std::endl;
        return false;
    }

    fs::path outputPath = ml::makePath(projectFolderPath.c_str(), templateResourceOutput);

    if (!ml::isExistParentPath(outputPath)) {
        std::cout << "Unable to copy resource at " << outputPath << " parent directory doesn't exist" << std::endl;
        return false;
    }

    std::error_code ec;
    fs::copy_options copyOptions = fs::copy_options::recursive;

    std::cout << "Copying template resource: " << templateResourceOutput << std::endl;

    fs::copy(templateResourcePath, outputPath, copyOptions, ec);

    if (ec) {
        std::cout << "Failed to copy resource: " << ec.message() << std::endl;
        return false;
    }

    return true;
}

static constexpr HandlerFunc Handlers[proj_creator::ResourceType_Count] = {
    &handleDirectory, // Directory
    &handleTemplateResource, // TemplateFile
    &handleTemplateResource // TemplateDirectory
};

// main/ui functions

std::string getProjectName() {
    std::string projectName;

    while (projectName.empty()) {
        std::cout << "Enter the name of the project: ";
        std::getline(std::cin, projectName);
    }

    return projectName;
}

int64_t getInputNumber(int64_t lowerBoundIncl, int64_t upperBoundIncl) {
    do {
        std::cout << "Enter a number between " << lowerBoundIncl << " and " << upperBoundIncl << ": ";

        std::string input;
        std::getline(std::cin, input);

        int64_t fallback = -1;
        int64_t inputNumber = ml::toInt64(input, fallback);

        if (mathl::inRangeIncl(inputNumber, lowerBoundIncl, upperBoundIncl)) {
            return inputNumber;
        }
    } while (true);
}

bool selectTemplate(fs::path* outTemplatePath) {
    fs::path templatesFolder = ml::fromCurrentPath("templates");

    if (!ml::isExistPath(templatesFolder)) {
        std::cout << "Templates folder not found: " << templatesFolder << std::endl;
        return false;
    }

    std::cout << "---- Template selection ----\n" << std::endl;

    int templateCount = 0;
    std::vector<fs::path> templatePaths;
    for (const fs::directory_entry& entry : fs::directory_iterator(templatesFolder)) {
        if (!fs::is_regular_file(entry)) {
            continue;
        }

        const fs::path& templatePath = entry.path();
        std::cout << ++templateCount << ": " << templatePath.filename().string() << std::endl;

        templatePaths.push_back(templatePath);
    }

    if (templateCount == 0) {
        std::cout << "No template files were found" << std::endl;
        return false;
    }

    std::cout << std::endl;

    int64_t templateNumber = getInputNumber(1, templateCount);

    *outTemplatePath = templatePaths[templateNumber - 1];

    return true;
}

bool parseLine(const std::string& line, const fs::path& destPath) {
    // # counts as a comment and is ignored
    if (line.starts_with('#')) {
        return false;
    }

    std::vector<std::string> tokens = ml::split(line, ':');

    if (!mathi::inRangeIncl(tokens.size(), 2, 3)) {
        std::cout << "Invalid line: " << line << ", format must be {resourceType:resourcePath:optional_output_path}" << std::endl;
        return false;
    }

    std::string resourceTypeStr = tokens[0];

    proj_creator::ResourceType resourceType;
    if (!proj_creator::ResourceType_FromString(&resourceType, resourceTypeStr.c_str())) {
        std::cout << "Invalid resource type: " << resourceTypeStr << std::endl;
        return false;
    }

    const std::string& resourceName = tokens[1];
    std::string templateResourceOutput = tokens.size() == 3
        ? tokens[2]
        : resourceName;

    HandlerFunc handler = Handlers[static_cast<int>(resourceType)];

    return handler(destPath, resourceName, templateResourceOutput);
}

bool createProject(const fs::path& destPath, const fs::path& templatePath) {
    std::ifstream file(templatePath);

    if (!file.is_open()) {
        return false;
    }

    bool createdSomething = false;

    std::string line;
    while (std::getline(file, line)) {
        if (parseLine(line, destPath)) {
            createdSomething = true;
        }
    }

    return createdSomething;
}

bool confirmProjectCreation(const fs::path& destPath) {
    if (!ml::isExistPath(destPath)) {
        std::cout << "You are about to create a project at " << destPath << ", continue? (y)" << std::endl;

        std::string input;
        std::getline(std::cin, input);

        return input == "y" && ml::createDirectory(destPath);
    }

    std::cout << destPath.filename() << " already exists, would you like to overwrite it? (y)" << std::endl;

    std::string input;
    std::getline(std::cin, input);

    if (input != "y") {
        return false;
    }

    if (ml::isExistPath(destPath)) {
        return ml::clearDirectory(destPath);
    }

    return true;
}

bool Config::load(const char* fileName) {
    fs::path configPath = fs::current_path();

    configPath.append(fileName);

    auto parser = [this](const std::string& key, const std::string& value) {
        if (key == "output") {
            mOutputFolder = value;
        }
    };

    return ml::parseConfig(configPath.string(), parser) && ml::isExistPath(mOutputFolder);
}
