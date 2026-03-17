#include "pch.h"
#include "Paths.h"

#include <Windows.h>
#include <shlobj.h>
#include <iostream>

namespace paths
{
    fs::path workingDir;

    fs::path assetsDir;
    fs::path shadersDir;
    fs::path modelsDir;
    fs::path blockModelsDir;
    fs::path texturesDir;
    fs::path blockTexturesDir;

    fs::path dependenciesDir;

    fs::path cacheDir;
    fs::path configDir;

    fs::path screenshotsDir;
    fs::path savesDir;
    fs::path logsDir;
    fs::path debugDir;
}

static fs::path GetWorkDirPath()
{
    PWSTR roamingPath = nullptr;

    HRESULT hr = SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, NULL, &roamingPath);
    if (FAILED(hr))
        throw std::runtime_error("Failed to get RoamingAppData path");

    fs::path path = fs::path(roamingPath) / ".mymox";

    CoTaskMemFree(roamingPath);

    fs::create_directories(path);

    std::cout << "Work dir: " << path << std::endl;

    return path;
}

static void EnsureDir(const fs::path& p)
{
    fs::create_directories(p);
}

void InitPaths()
{
    using namespace paths;

    workingDir = GetWorkDirPath();

    // core
    assetsDir = workingDir / "assets";
    dependenciesDir = workingDir / "dependencies";
    cacheDir = workingDir / "cache";
    configDir = workingDir / "config";

    // assets
    shadersDir = assetsDir / "shaders";
    modelsDir = assetsDir / "models";
    texturesDir = assetsDir / "textures";

    blockModelsDir = modelsDir / "blocks";
    blockTexturesDir = texturesDir / "blocks";

    // runtime
    screenshotsDir = workingDir / "screenshots";
    savesDir = workingDir / "saves";
    logsDir = workingDir / "logs";
    debugDir = workingDir / "debug";

    // create dirs
    for (const auto& dir :
        {
            assetsDir,
            shadersDir,
            modelsDir,
            blockModelsDir,
            texturesDir,
            blockTexturesDir,

            dependenciesDir,
            cacheDir,
            configDir,

            screenshotsDir,
            savesDir,
            logsDir,
            debugDir
        })
    {
        EnsureDir(dir);
    }
}