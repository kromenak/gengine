//
// Model.h
//
// Clark Kromenaker
//
// 3D model asset type. The in-memory
// representation of .MOD assets.
//
#pragma once
#include <string>
#include <vector>
#include "Asset.h"

class Mesh;

class Model : public Asset
{
public:
    Model(std::string name, char* data, int dataLength);
    
    std::vector<Mesh*> GetMeshes() const { return mMeshes; }
    
private:
    // A model consists of one or more meshes.
    std::vector<Mesh*> mMeshes;
    
    void ParseFromData(char* data, int dataLength);
};
