//
// Clark Kromenaker
//
// GK3 3D model asset type. The in-memory representation of .MOD assets.
//
// A Model is not actually a renderable object - it just contains one or more
// Meshes, which themselves contain one or more Submeshes. Think of Model as
// the container for the renderable data, like an .FBX or Maya file.
//
// However, Models are fairly important concepts in GK3 because the vertex animation
// technique requires animations to be exactly matched to particular models.
//
#pragma once
#include "Asset.h"

#include <string>
#include <vector>

class Mesh;

class Model : public Asset
{
public:
    Model(const std::string& name, AssetScope scope) : Asset(name, scope) { }
    ~Model();

    void Load(uint8_t* data, uint32_t dataLength);

    const std::vector<Mesh*>& GetMeshes() const { return mMeshes; }
	
	bool IsBillboard() const { return mBillboard; }
	
	void WriteToObjFile(const std::string& filePath);
	
private:
    // A model consists of one or more meshes.
    std::vector<Mesh*> mMeshes;
	
	// If true, the model should be rendered as a billboard.
	bool mBillboard = false;
	
    void ParseFromData(uint8_t* data, uint32_t dataLength);
};
