#include "BSPLightmap.h"

#include "BinaryReader.h"
#include "Texture.h"

BSPLightmap::~BSPLightmap()
{
    // This class owns the textures created in the constructor, so we must delete them.
    for(auto& texture : mLightmapTextures)
    {
        delete texture;
    }
}

void BSPLightmap::Load(uint8_t* data, uint32_t dataLength)
{
    BinaryReader reader(data, dataLength);

    // 4 bytes: file identifier "TULM" (MULT backwards).
    std::string identifier = reader.ReadString(4);
    if(identifier != "TLUM")
    {
        std::cout << "BSP lightmap asset does not have MULT identifier! Instead has " << identifier << std::endl;
        return;
    }

    // 4 bytes: number of bitmaps in this asset.
    // This value correlates to the number of BSP surfaces in the corresponding BSP asset.
    unsigned int bitmapCount = reader.ReadUInt();

    // Iterate and read in each bitmap in turn.
    for(unsigned int i = 0; i < bitmapCount; i++)
    {
        // The texture will be read in using the same reader object.
        // This should leave the reader ready to read in the NEXT texture (assuming no texture parsing bugs).
        Texture* texture = new Texture(reader);
        texture->SetFilterMode(Texture::FilterMode::Bilinear);
        texture->SetWrapMode(Texture::WrapMode::Clamp);
        mLightmapTextures.push_back(texture);
    }

    /*
    // Write out for debugging...
    for(int i = 0; i < mLightmapTextures.size(); i++)
    {
        mLightmapTextures[i]->WriteToFile(GetNameNoExtension() + "_lm_" + std::to_string(i) + ".bmp");
    }
    */
}

