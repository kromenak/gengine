//
// FileSystem.cpp
//
// Clark Kromenaker
//
#include "FileSystem.h"

#include "Platform.h"

#if defined(PLATFORM_MAC)
#include <CoreFoundation/CoreFoundation.h>
#endif

#include <fstream>

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
