#include "Asset.h"

#include "FileSystem.h"

Asset::Asset(const std::string& name, AssetScope scope) :
    mName(name),
    mScope(scope)
{
    
}

std::string Asset::GetNameNoExtension() const
{
    return Path::RemoveExtension(mName);
}
