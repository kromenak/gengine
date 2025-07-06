#include "Asset.h"

#include "FileSystem.h"

TYPEINFO_INIT(Asset, NoBaseClass, 100)
{
    TYPEINFO_VAR(Asset, VariableType::String, mName);
}

Asset::Asset(const std::string& name, AssetScope scope) :
    mName(name),
    mScope(scope)
{

}

std::string Asset::GetNameNoExtension() const
{
    return Path::RemoveExtension(mName);
}
