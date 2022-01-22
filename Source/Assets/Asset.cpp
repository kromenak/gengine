#include "Asset.h"

#include "FileSystem.h"

Asset::Asset(const std::string& name) : mName(name)
{
    
}

std::string Asset::GetNameNoExtension()
{
    return Path::RemoveExtension(mName);
}
