//
// FileSystem.h
//
// Clark Kromenaker
//
// Functions to perform platform-specific file system operations.
//
#pragma once
#include <iostream>
#include <string>

#include <dirent.h>
#include <sys/stat.h>

#include "StringTokenizer.h"

namespace Path
{
	// Separator used for platform.
	const char kSeparator = '/';
	
	std::string Combine(std::initializer_list<std::string> paths);
}

namespace Directory
{
	/**
	 * Returns true if the directory at path exists, false if it doesn't.
	 */
	inline bool Exists(std::string path)
	{
		DIR* directoryStream = opendir(path.c_str());
		if(directoryStream == nullptr)
		{
			//TODO: Detect whether the directory doesn't exist, or an error occurred.
			//TODO: If an error occurred, we don't know for sure whether the directory exists or not.
			return false;
		}
		closedir(directoryStream);
		return true;
	}
	
	/**
	 * Creates the directory at path. Fails if the directory already exists.
	 *
	 * Does not create intermediate/parent directories (see MakeAllDirectories for that).
	 * Will fail if parent directory is missing.
	 *
	 * Returns true on successful creation (or if directory already exists).
	 * Returns false if an error occurred.
	 */
	inline bool Create(std::string path)
	{
		// Makes the directory with Read/Write/Execute permissions for User and Group, Read/Execute for Other.
		const int result = mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
		
		// A non-zero result indicates an error.
		if(result != 0)
		{
			// If error is that directory already exists...great, wonderful, ok!
			if(errno == EEXIST) { return true; }
			
			// Some error occurred.
			std::cout << "Failed to make directory at " << path << std::endl;
			return false;
		}
		return true;
	}
	
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
