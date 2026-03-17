#pragma once

#include <filesystem>

namespace fs = std::filesystem;

namespace paths
{
    extern fs::path workingDir;

    extern fs::path assetsDir;
    extern fs::path shadersDir;
    extern fs::path modelsDir;
    extern fs::path blockModelsDir;
    extern fs::path texturesDir;
    extern fs::path blockTexturesDir;

    extern fs::path dependenciesDir;

    extern fs::path cacheDir;
    extern fs::path configDir;

    extern fs::path screenshotsDir;
    extern fs::path savesDir;
    extern fs::path logsDir;
    extern fs::path debugDir;
}

void InitPaths();