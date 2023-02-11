#include "Paths.h"

#include <SDL.h>

#include "FileSystem.h"

const std::string& Paths::GetSaveDataPath()
{
    static std::string saveDataPath;
    if(saveDataPath.empty())
    {
        // Obtain writable preferences/savedata directory.
        char* path = SDL_GetPrefPath("Sierra On-Line", "Gabriel Knight 3");
        saveDataPath.assign(path);

        // SDL always returns paths with a trailing separator, but we don't want that.
        saveDataPath.pop_back();

        // Free char memory (pointer returned by GetPrefPath is owned by us).
        SDL_free(path);
    }
    return saveDataPath;
}

std::string Paths::GetSaveDataPath(const std::string& filename)
{
    return Path::Combine({ GetSaveDataPath(), filename });
}

const std::string& Paths::GetDataPath()
{
    static std::string dataPath;
    if(dataPath.empty())
    {
        // Obtain read-only data path.
        char* path = SDL_GetBasePath();
        dataPath.assign(path);

        // SDL always returns paths with a trailing separator, but we don't want that.
        dataPath.pop_back();

        // Free char memory (pointer returned by GetPrefPath is owned by us).
        SDL_free(path);
    }
    return dataPath;
}

std::string Paths::GetDataPath(const std::string& filename)
{
    return Path::Combine({ GetDataPath(), filename });
}