//
// FileSystem.cpp
//
// Clark Kromenaker
//
#include "FileSystem.h"

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
