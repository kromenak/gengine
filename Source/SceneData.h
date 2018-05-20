//
// SceneData.h
//
// Clark Kromenaker
//
// An asset that mostly just indicates the BSP and light positions for
// a scene in the game.
//
// In-memory representation of a .SCN asset.
//
#pragma once
#include "Asset.h"

class SceneData : public Asset
{
public:
    SceneData(std::string name, char* data, int dataLength);
    
    std::string GetBSPName();
    
private:
    std::string mBSPNameOverride;
    
    void ParseFromData(char* data, int dataLength);
};
