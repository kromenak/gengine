//
// FileSystem.cpp
//
// Clark Kromenaker
//
#include "FileSystem.h"

#include <fstream>

#include "Platform.h"
#if defined(PLATFORM_MAC)
#include <CoreFoundation/CoreFoundation.h>
#include <dirent.h>
#include <sys/stat.h>
#elif defined(PLATFORM_WINDOWS)
#include <Windows.h>
#endif

std::string Path::Combine(std::initializer_list<std::string> paths)
{
	// Can't combine zero paths!
	if(paths.size() == 0) { return std::string(); }
	
	// Start the combined path with the first path piece.
	auto it = paths.begin();
	std::string combined = *it;
	it++;
	
	// Append each path piece to the combined value.
	while(it != paths.end())
	{
		// Skip empty pieces.
		if(it->empty()) { continue; }
		
		// Each piece is separated by the path separator ('/' or '\').
		combined += kSeparator;
		combined += *it;
		
		// Move to next element.
		it++;
	}
	return combined;
}

bool Path::FindFullPath(const std::string& fileName, const std::string& relativeSearchPath, std::string& outPath)
{
#if defined(PLATFORM_MAC)
	// Get ref to the main bundle. This is the ".app" bundle the app is executing in.
	// Even if NOT running in a bundle (like as a command line tool), this still works - OS just "pretends" the directory of the app is the bundle root.
	// Supposedly though, this can be null in some case...so gotta check.
	CFBundleRef bundleRef = CFBundleGetMainBundle();
	if(bundleRef != nullptr)
	{
		// We're in Apple land...gotta use CFStrings.
		CFStringRef searchPathCFStr = CFStringCreateWithCString(CFAllocatorGetDefault(), relativeSearchPath.c_str(), kCFStringEncodingUTF8);
		CFStringRef fileNameCFStr = CFStringCreateWithCString(CFAllocatorGetDefault(), fileName.c_str(), kCFStringEncodingUTF8);
		
		// Moment of truth: ask for the resource's URL, using the file name and search path.
		// The OS does a bunch of stuff to determine whether this asset exists and return it.
		CFURLRef resourceUrl = CFBundleCopyResourceURL(bundleRef, fileNameCFStr, NULL, searchPathCFStr);
		if(resourceUrl != nullptr)
		{
			// Converts the URL to a string that can be used to read from the file system.
			// The URL probably is like "file://Dir1/Dir2/File.txt", while the string is like "/Dir1/Dir2/File.txt"
			CFStringRef resourceUrlStr = CFURLCopyFileSystemPath(resourceUrl, kCFURLPOSIXPathStyle);
			outPath = std::string(CFStringGetCStringPtr(resourceUrlStr, kCFStringEncodingUTF8));
			CFRelease(resourceUrlStr);
		}
		
		CFRelease(searchPathCFStr);
		CFRelease(fileNameCFStr);
		
		// File exists if resource URL is not null.
		return resourceUrl != nullptr;
	}
	//NOTE: if can't get a bundle ref, we purposely drop through to "failsafe" method below.
#endif
	
	// Worst case, if no platform-specific way to get a file path, we can just combine the search path and filename.
	// This'll probably give us something like "Assets/File.txt"
	// C++ is able to load relative files, assuming the current working directory (cwd) is as expected.
	outPath = relativeSearchPath + fileName;
	
	// How can you tell if the path is valid? Well, you've gotta see if you can open a stream!
	std::ifstream f(outPath.c_str());
	if(f.good()) { return true; }
	
	// Failure!
	return false;
}

std::string Path::GetFileName(const std::string& path)
{
	// Make sure there's any content in the path argument.
	if(path.empty()) { return path; }
	
	// Find the last index of the separator character.
	size_t pos = path.find_last_of(kSeparator);
	
	// If no separator exists, just return the path as-is...
	// it's either invalid or a single element like "MyFile".
	if(pos == std::string::npos)
	{
		return path;
	}
	
	// If pos is at end of string, return empty string.
	// Something like "/Users/Bob/" should return "".
	if(pos + 1 >= path.size())
	{
		return "";
	}
	
	// Get everything after the separator and return it
	return path.substr(pos + 1, std::string::npos);
}

std::string Path::GetFileNameNoExtension(const std::string& path)
{
	// Get filename with extension first.
	std::string filename = GetFileName(path);
	
	// Find extension separator, if any.
	size_t pos = filename.find_last_of('.');
	
	// If no extension, we can just return the filename as-is.
	if(pos == std::string::npos)
	{
		return filename;
	}
	
	// Return everything before the extension separator.
	return filename.substr(0, pos);
}

bool Directory::Exists(const std::string& path)
{
#if defined(PLATFORM_MAC)
	DIR* directoryStream = opendir(path.c_str());
	if (directoryStream == nullptr)
	{
		//TODO: Detect whether the directory doesn't exist, or an error occurred.
		//TODO: If an error occurred, we don't know for sure whether the directory exists or not.
		return false;
	}
	closedir(directoryStream);
	return true;
#elif defined(PLATFORM_WINDOWS)
	DWORD fileAttributes = GetFileAttributesA(path.c_str());

	// In this case, the provided path might be malformed.
	if (fileAttributes == INVALID_FILE_ATTRIBUTES) { return false; }

	// If attribute has directory flag, it is a directory and it does exist!
	if (fileAttributes & FILE_ATTRIBUTE_DIRECTORY != 0) { return true; }

	// This is not a directory.
	return false;
#endif
}

bool Directory::Create(const std::string& path)
{
#if defined(PLATFORM_MAC)
	// Makes the directory with Read/Write/Execute permissions for User and Group, Read/Execute for Other.
	const int result = mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

	// A non-zero result indicates an error.
	if (result != 0)
	{
		// If error is that directory already exists...great, wonderful, ok!
		if (errno == EEXIST) { return true; }

		// Some error occurred.
		std::cout << "Failed to make directory at " << path << std::endl;
		return false;
	}
	return true;
#elif defined (PLATFORM_WINDOWS)
	// Make the directory.
	bool result = CreateDirectory(path.c_str(), NULL);

	// A false result indicates an error.
	if (!result)
	{
		// If error is that directory already exists...great, wonderful, ok!
		if (GetLastError() == ERROR_ALREADY_EXISTS) { return true; }

		// Some error occurred.
		std::cout << "Failed to make directory at " << path << std::endl;
		return false;
	}
	return true;
#endif
}
