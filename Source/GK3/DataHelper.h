//
// Clark Kromenaker
//
// Helps determine what Data directories are available and which one should be used to run the game.
//
#pragma once
#include <string>

struct DataDirectory
{
    // The relative path to this directory.
    std::string path;

    // The detected localization name for this data.
    std::string localeName;

    // The detected localization prefix for this data.
    char localePrefix;
};

namespace DataHelper
{
    bool GetDataDirectoryToUse(DataDirectory& dataDirectory);
}