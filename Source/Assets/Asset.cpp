//
// Asset.cpp
//
// Clark Kromenaker
//
#include "Asset.h"

#include "FileSystem.h"

Asset::Asset(std::string name) : mName(name)
{
    
}

std::string Asset::GetNameNoExtension()
{
    return File::RemoveExtension(mName);
}
