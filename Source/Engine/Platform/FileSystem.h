//
// Clark Kromenaker
//
// Functions to perform platform-specific file system operations.
//
#pragma once
#include <cstdint>
#include <iostream>
#include <string>

#include "Platform.h"
#include "StringTokenizer.h"

namespace Path
{
    // Separator used for platform.
    #if defined(PLATFORM_WINDOWS)
    const char kSeparator = '\\';
    #else
    const char kSeparator = '/';
    #endif

    /**
     * Combines pieces of path together using the appropriate path separator.
     * Ex: "/Applications/GK3.app" and "Contents/Blah.png" becomes "/Applications/GK3.app/Contents/Blah.png".
     */
    std::string Combine(std::initializer_list<std::string> paths);


    /**
     * Given a filename and a relative search path, determine if a file exists (return value) and determines
     * a full path (via out variable) that can be used to read the file.
     *
     * If a file is in a sub-directory next to the executable, this is fairly trivial (though determining if the file exists is still a bit iffy).
     * But on some platforms (like OSX), getting a resource that exists in the app bundle is not entirely straightforward.
     */
    bool FindFullPath(const std::string& fileName, const std::string& relativeSearchPath, std::string& outPath);

    /**
     * Given a path, returns the name of the file only.
     * Ex: "/Projects/Project/Assets/MyAsset.shp" becomes "MyAsset.shp".
     * Ex: "/Projects/Project/" becomes "".
     */
    std::string GetFileName(const std::string& path);

    /**
     * Similar to GetFileName, but returns file name with no extension (everything after and including '.' is removed).
     * Ex: "/Projects/Project/Assets/MyAsset.shp" becomes "MyAsset".
     */
    std::string GetFileNameNoExtension(const std::string& path);

    /**
     * Given a file name or path, returns true if an extension is present.
     * If an expected extension is provided, only returns true if THAT SPECIFIC extension is present.
     * Any path with a "." in the last token is considered to have an extension.
     * Ex: "Assets/Data/Blah" has no extension, "Assets/Data/Blah.x" does have an extension.
     */
    bool HasExtension(const std::string& path, const std::string& expectedExtension = "");

    /**
     * Returns just the extension for the file in the path, or empty string if no extension exists.
     * If desired, can optionally include the ".", but that is excluded by default.
     * Ex: "Assets/Data/Test.png" would return "png" (or ".png" if you include the dot).
     */
    std::string GetExtension(const std::string& path, bool includeDot = false);

    /**
     * Applies an extension to the given path.
     * If an extension already exists, it is REPLACED with the desired extension.
     */
    std::string SetExtension(const std::string& path, const std::string& extension);

    /**
     * Removes the extension from the path.
     * Just returns a copy of the current file name if no extension is present.
     */
    inline std::string RemoveExtension(const std::string& path)
    {
        return HasExtension(path) ? path.substr(0, path.find_last_of('.')) : path;
    }

    /**
     * Detects whether a path is absolute.
     */
    inline bool IsAbsolute(const std::string& path)
    {
        #if defined(PLATFORM_WINDOWS)
        // On Windows, a path must start with a drive letter to be absolute (e.g. "C:\").
        // Windows also supports relative paths from root of drive (starting with "\"). Going to NOT count those for now.
        return path.size() >= 3 && path[0] >= 'A' && path[0] <= 'Z' && path[1] == ':' && path[2] == '\\';
        #else
        // On Mac/Linux, pretty much just need the leading character to be the path separator.
        return !path.empty() && path[0] == '/';
        #endif
    }
}

namespace Directory
{
    /**
     * Returns true if the directory exists, false if it doesn't.
     */
    bool Exists(const std::string& path);

    /**
     * Creates a directory.
     *
     * Does not create intermediate/parent directories (see CreateAll for that).
     * Will fail if parent directory is missing.
     *
     * Returns true on successful creation (or if directory already exists).
     * Returns false if an error occurred.
     */
    bool Create(const std::string& path);

    /**
     * Creates one or more directories in a given path.
     *
     * If making foo/bar/xyz, and none exist, each will be created in turn.
     */
    bool CreateAll(const std::string& path);

    /**
     * Lists the files in a directory, optionally filtering to only files with a specific extension.
     */
    std::vector<std::string> List(const std::string& path, const std::string& extension = "");
}

namespace File
{
    /**
     * Checks if the file exists.
     */
    bool Exists(const std::string& filePath);

    /**
     * Determines a file's size in bytes.
     */
    uint64_t Size(const std::string& filePath);

    /**
     * Reads file contents into a buffer.
     */
    uint8_t* ReadIntoBuffer(const std::string& filePath, uint32_t& outBufferSize);
}