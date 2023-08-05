//
// Clark Kromenaker
//
// A lightmap for BSP geometry. Contains an array of textures meant to
// be applied to BSP surfaces to give the appearance of light and shadows.
//
// Each lightmap is meant for a specific BSP geometry. A BSP may have multiple
// lightmaps (e.g. a lightmap for morning, one for evening, one for night).
//
// In-memory representation of .MUL files. The MUL file format is basically
// a blob containing one or more BMP files.
//
#pragma once
#include "Asset.h"

#include <string>
#include <vector>

class Texture;

class BSPLightmap : public Asset
{
public:
    BSPLightmap(const std::string& name, AssetScope scope) : Asset(name, scope) { }
    ~BSPLightmap();

    void Load(uint8_t* data, uint32_t dataLength);
    
    const std::vector<Texture*>& GetLightmapTextures() const { return mLightmapTextures; }
    
private:
    // Textures loaded from the MUL file.
    // Order is important, and aligns with order of surfaces in BSP file.
    // Unlike most Textures, this asset owns these Textures, and is responsible for cleanup!
    std::vector<Texture*> mLightmapTextures;
};
