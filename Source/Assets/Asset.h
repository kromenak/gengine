//
// Clark Kromenaker
//
// Base class for any game asset.
// An asset is any game content loaded from the disk - textures, meshes, audio, scripts, etc.
//
#pragma once
#include <string>

class Asset
{
public:
    Asset(const std::string& name);
    virtual ~Asset() { }
    
    const std::string& GetName() { return mName; }
    std::string GetNameNoExtension();
    
protected:
    std::string mName;
};
