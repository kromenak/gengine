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

class Model : Asset
{
public:
    Model(std::string name, char* data, int dataLength);
    
    std::vector<Mesh*> GetMeshes() const { return mMeshes; }
    std::vector<std::string> GetTextureNames() const { return mTextureNames; }
    
private:
    // A model consists of one or more meshes.
    std::vector<Mesh*> mMeshes;
    
    // Each mesh in a model has a "name" that correlates to the
    // name of the texture it should use when rendering.
    // There is one entry in this array for each entry in the meshes array.
    // It is possible for an entry to be empty - some meshes have no name.
    std::vector<std::string> mTextureNames;
    
    void ParseFromData(char* data, int dataLength);
};
