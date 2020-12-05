//
//  BarnAsset.h
//  GEngine
//
//  Created by Clark Kromenaker on 8/6/17.
//
#pragma once
#include <string>

enum class CompressionType
{
    None = 0,
    Zlib = 1,
    Lzo = 2
};

class BarnAsset
{
public:
    // Name of barn file containing this asset.
    // If not empty, it means this Asset handle is a pointer to another barn file.
    std::string barnFileName;
    
    // The name of the asset itself.
    std::string name;
    
    // Offset of this asset within the Barn file data blob.
    unsigned int offset = 0;
    
    // If the asset is compressed, and what it's compressed size is.
    CompressionType compressionType = CompressionType::None;
    unsigned int compressedSize = 0;
    
    // The uncompressed size of the asset. For assets that aren't compressed,
    // compressed and uncompressed size are the same!
    unsigned int uncompressedSize = 0;
    
    // True if this BarnAsset is just a pointer to another barn file.
    bool IsPointer() { return !barnFileName.empty(); }
};
