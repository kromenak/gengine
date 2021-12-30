//
// Clark Kromenaker
//
// Functions to perform platform-specific file system operations.
//
#pragma once
#include <iostream>
#include <string>

#include "Atomics.h"
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
     * Any path with a "." in the last token is considered to have an extension.
     * Ex: "Assets/Data/Blah" has no extension, "Assets/Data/Blah.x" does have an extension.
     */
    bool HasExtension(const std::string& path);

    /**
     * Removes the extension from the file name.
     * Just returns a copy of the current file name if no extension is present.
     */
    inline std::string RemoveExtension(const std::string& fileName) { return fileName.substr(0, fileName.find_last_of('.')); }
}

namespace Directory
{
	/**
	 * Returns true if the directory at path exists, false if it doesn't.
	 */
	bool Exists(const std::string& path);
	
	/**
	 * Creates the directory at path. Fails if the directory already exists.
	 *
	 * Does not create intermediate/parent directories (see MakeAllDirectories for that).
	 * Will fail if parent directory is missing.
	 *
	 * Returns true on successful creation (or if directory already exists).
	 * Returns false if an error occurred.
	 */
	bool Create(const std::string& path);
	
	/**
	 * Makes one or more directories in a given path.
	 *
	 * If making foo/bar/xyz, and none exist, each will be created in turn.
	 */
	inline bool CreateAll(std::string path)
	{
		StringTokenizer tokenizer(path, { '/' });
		std::string buildPath;
		while(tokenizer.HasNext())
		{
			buildPath += tokenizer.GetNext();
			
			bool madeDirectory = Create(buildPath);
			if(!madeDirectory) { return false; }
			
			buildPath += Path::kSeparator;
		}
		return true;
	}
}

namespace File
{
    /**
     * Determines a file's size in bytes.
     */
    int64 Size(const std::string& filePath);

    /**
     * Reads file contents into a buffer.
     */
    char* ReadIntoBuffer(const std::string& filePath, uint32& outBufferSize);
}
