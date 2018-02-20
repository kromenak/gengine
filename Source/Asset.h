//
// Asset.h
//
// Clark Kromenaker
//
// Base class for any game asset.
// An asset is any game content loaded from the disk
// - textures, meshes, audio, scripts, etc.
//
#pragma once
#include <string>

class Asset
{
public:
    Asset(std::string name);
    
    std::string GetName() { return mName; }
    
protected:
    std::string mName;
};
