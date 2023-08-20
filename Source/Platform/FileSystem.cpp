#include "FileSystem.h"

#include <fstream>

#include "StringUtil.h"

#if defined(PLATFORM_MAC)
#include <CoreFoundation/CoreFoundation.h>
#elif defined(PLATFORM_WINDOWS)
#include <Windows.h>
#endif

#if defined(HAVE_DIRENT_H)
#include <dirent.h>
#endif

#if defined(HAVE_STAT_H)
#include <sys/stat.h>
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

        // Place path separator ('/' or '\') between elements.
        // Empty check stops us from putting separators at beginning of path accidentally.
        if(!combined.empty())
        {
            combined += kSeparator;
        }

        // Add path piece.
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
        //TODO: This call is case-sensitive, even if OSX is not case-sensitive! Ideally, this should be case-insensitive like the OS is.
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
        if(resourceUrl != nullptr)
        {
            return true;
        }
	}
	//NOTE: if can't get a bundle ref or resource url, we purposely drop through to "failsafe" method below.
    #endif
	
	// Worst case, if no platform-specific way to get a file path, we can just combine the search path and filename.
	// This'll probably give us something like "Assets/File.txt"
	// C++ is able to load relative files, assuming the current working directory (cwd) is as expected.
    outPath = Path::Combine({ relativeSearchPath, fileName });

    // Use some method to determine if the file exists or not.
    #if defined(HAVE_STAT_H)
    // On systems that have stat.h header, we can use this method. This is technically POSIX-only, but Windows has the header too...
    // Some people on StackOverflow found this to be faster than other methods: https://stackoverflow.com/a/12774387/782181
    struct stat buffer;
    return (stat(outPath.c_str(), &buffer) == 0);
    #else
    // Using just standard C++, we can tell if a file exists by opening a stream and seeing if it works.
	std::ifstream f(outPath.c_str());
    return f.good();
    #endif
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

bool Path::HasExtension(const std::string& path, const std::string& expectedExtension)
{
    // If empty, no extension.
    if(path.empty()) { return false; }

    // Having an expected extension can actually make this easier.
    // Just make sure the path ends with the expected extension.
    if(!expectedExtension.empty())
    {
        // If the expected extension includes the dot, just make sure the path ends with this extension.
        if(expectedExtension[0] == '.')
        {
            return StringUtil::EndsWithIgnoreCase(path, expectedExtension);
        }
        else // expected extension doesn't include a dot
        {
            // We still need the path to end with the expected extension.
            // However, we also need to verify that the character before the expected extension is a dot in the path!
            return path.size() > expectedExtension.size() &&
                path[path.size() - expectedExtension.size() - 1] == '.' &&
                StringUtil::EndsWithIgnoreCase(path, expectedExtension);
        }        
    }
    else // No expected extension - we just need to verify ANY extension is present.
    {
        // Find last dot in path.
        size_t lastExtensionPos = path.find_last_of('.');

        // No period in the string? Guess we have no extension.
        if(lastExtensionPos == std::string::npos)
        {
            return false;
        }

         // Need to make sure last '.' is in the last part of the path.
        // "Assets/Foo.proj/Blah" is not considered to have an extension, for example.
        size_t lastSeparatorPos = path.find_last_of(kSeparator);

        // We definitely have an extension period in the path to get here.
        // So, if no separator exists, an extension exists. Or, if last separator is before extension period, an extension exists.
        return (lastSeparatorPos == std::string::npos || lastSeparatorPos < lastExtensionPos);
    }
}

bool Directory::Exists(const std::string& path)
{
    #if defined(PLATFORM_WINDOWS)
    {
        DWORD fileAttributes = GetFileAttributesA(path.c_str());

        // In this case, the provided path might be malformed.
        if(fileAttributes == INVALID_FILE_ATTRIBUTES) { return false; }

        // If attribute has directory flag, it is a directory and it does exist!
        if((fileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) { return true; }

        // This is not a directory.
        return false;
    }
    #elif defined(HAVE_DIRENT_H)
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
    #else
        #error "No implementation for Directory::Exists!"
    #endif
}

bool Directory::Create(const std::string& path)
{
    #if defined(PLATFORM_WINDOWS)
    {
        // Make the directory.
        bool result = CreateDirectory(path.c_str(), NULL);

        // A false result indicates an error.
        if(!result)
        {
            // If error is that directory already exists...great, wonderful, ok!
            if(GetLastError() == ERROR_ALREADY_EXISTS) { return true; }

            // Some error occurred.
            std::cout << "Failed to make directory at " << path << std::endl;
            return false;
        }
        return true;
    }
    #elif defined(HAVE_STAT_H)
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
    #else
        #error "No implementation for Directory::Create!"
        return false;
    #endif
}

uint64_t File::Size(const std::string& filePath)
{
    #if defined(PLATFORM_WINDOWS)
    {
        // Use Windows function to get attributes and return size.
        WIN32_FILE_ATTRIBUTE_DATA file_attr_data;
        if(GetFileAttributesEx(filePath.c_str(), GetFileExInfoStandard, &file_attr_data))
        {
            // For compatibility reasons, the size is stored as two 32-bit ints, but it's meant to represent a 64-bit int.
            // Can use the LARGE_INTEGER struct to convert to int64.
            ULARGE_INTEGER fileSize = { 0 };
            fileSize.LowPart = file_attr_data.nFileSizeLow;
            fileSize.HighPart = file_attr_data.nFileSizeHigh;
            return fileSize.QuadPart;
        }
    }
    #elif defined(HAVE_STAT_H)
    {
        // This should work on Mac/Linux. (it may even work on Windows, depending on version)
        struct stat stat_buf;
        int rc = stat(filePath.c_str(), &stat_buf);
        if(rc == 0)
        {
            return stat_buf.st_size;
        }
    }
    #else
        #error "No implementation for File::Size!"
    #endif

    // Failed to get size, so just return 0.
    return 0;
}

uint8_t* File::ReadIntoBuffer(const std::string& filePath, uint32_t& outBufferSize)
{
    // Open the file, or error if failed.
    std::ifstream file(filePath, std::iostream::in | std::iostream::binary);
    if(!file.good())
    {
        outBufferSize = 0;
        return nullptr;
    }

    // Get size of file, so we can make a buffer for its contents.
    uint64_t size = File::Size(filePath);

    // Create buffer and read in data.
    // This may be a binary or text asset. But to be on the safe side, let's stick a null terminator on there.
    uint8_t* buffer = new uint8_t[size + 1];
    file.read(reinterpret_cast<char*>(buffer), size);
    buffer[size] = '\0';

    // Pass out buffer size and return buffer.
    outBufferSize = size + 1;
    return buffer;
}