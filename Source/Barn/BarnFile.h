//
//  BarnFile.h
//  GEngine
//
//  Created by Clark Kromenaker on 8/4/17.
//

#pragma once

enum class AssetCompressionType
{
    None = 0,
    Zlib = 1,
    Lzo = 2
};

class BarnFile
{
    
    
public:
    BarnFile(const char* filePath);
    
private:
    const int kDirectoryIdentifier = 0x44446972; // DDir
    const int kDataIdentifier = 0x44617461; // Data
    
    unsigned int mDataOffset = 0;
};
