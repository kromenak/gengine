//
// Clark Kromenaker
//
// Helper to retrieve important paths for the operation of the game.
//
#pragma once
#include <string>

namespace Paths
{
    /**
     * The "save data path" is a writable directory for preferences and save games.
     */
    const std::string& GetSaveDataPath();
    std::string GetSaveDataPath(const std::string& filename);

    /**
     * The "data path" is the root directory for the game's read-only data (Assets, Data, etc).
     * This is usually the directory containing the executable, but not always (on Mac/iOS for example, this is "GK3.app/Contents/Resources").
     */
    const std::string& GetDataPath();
    std::string GetDataPath(const std::string& filename);
};