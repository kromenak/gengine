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
    std::string barnFileName = "";
    
    std::string name = "";
    
    unsigned int offset = 0;
    
    CompressionType compressionType = CompressionType::None;
    unsigned int compressedSize = 0;
    
    unsigned int uncompressedSize = 0;
};
