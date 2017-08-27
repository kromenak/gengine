//
//  BarnFile.h
//  GEngine
//
//  Created by Clark Kromenaker on 8/4/17.
//

#pragma once
#include "BarnAsset.h"
#include "BinaryReader.h"
#include <unordered_map>

class BarnFile
{
public:
    BarnFile(const char* filePath);
    
    bool CanRead() const;
    
    void OutputAssetList();
    
    BarnAsset* GetAsset(const std::string assetName);
    
    bool Extract(const std::string assetName, char* buffer, int bufferSize);
    
    bool WriteToFile(const std::string assetName);
    
private:
    const int kGameIdentifier = 0x21334B47; // GK3!
    const int kBarnIdentifier = 0x6E726142; // Barn
    
    const int kDDirIdentifier = 0x44446972; // DDir
    const int kDataIdentifier = 0x44617461; // Data
    
    // Binary reader for extracting data.
    BinaryReader mReader;
    
    // Offset within the file to where the data is located.
    unsigned int mDataOffset = 0;
    
    // Map of asset name to an asset handle.
    // The asset needs to be extracted before it can be used.
    std::unordered_map<std::string, BarnAsset> mAssetMap;
};
