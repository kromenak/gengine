//
// Scene.h
//
// Clark Kromenaker
//
// An asset that mostly just indicates the BSP and light positions for
// a stage in the game.
//
// In-memory representation of a .SCN asset.
//
#pragma once
#include "Asset.h"

class Scene : public Asset
{
public:
    Scene(std::string name, char* data, int dataLength);
    
private:
    void ParseFromData(char* data, int dataLength);
};
