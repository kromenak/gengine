#include "Asset.h"

#include "FileSystem.h"

Asset::Asset(std::string name) : mName(name)
{
    
}

std::string Asset::GetNameNoExtension()
{
    return Path::RemoveExtension(mName);
}
