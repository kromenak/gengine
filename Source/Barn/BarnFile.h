//
//  BarnFile.h
//  GEngine
//
//  Created by Clark Kromenaker on 8/4/17.
//
#pragma once
#include <string>
#include <unordered_map>

#include "BarnAsset.h"
#include "BinaryReader.h"

class BarnFile
{
public:
    BarnFile(const std::string& filePath);
	
	// Ensure we can actually read assets from this barn.
    bool CanRead() const;
	
	// Retrieves an asset handle, if it exists in this bundle.
    BarnAsset* GetAsset(const std::string& assetName);
	
	// Extracts an asset into the provided buffer.
    bool Extract(const std::string& assetName, char* buffer, int bufferSize);
	
	// For debugging, write assets to file.
    bool WriteToFile(const std::string& assetName);
	bool WriteToFile(const std::string& assetName, const std::string outputDir);
	
	// For debugging, write assets to file whose names match a search string.
	void WriteAllToFile(const std::string& search);
	void WriteAllToFile(const std::string& search, const std::string outputDir);
	
	// For debugging, output asset list to cout.
	void OutputAssetList() const;
    
private:
	// Identifiers required to verify file type.
    const int kGameIdentifier = 0x21334B47; // GK3!
    const int kBarnIdentifier = 0x6E726142; // Barn
	
	// Identifiers required to identify data section.
    const int kDDirIdentifier = 0x44446972; // DDir
    const int kDataIdentifier = 0x44617461; // Data
    
    // The name of the barn file.
    std::string mName;
    
    // Binary reader for extracting data.
    BinaryReader mReader;
    
    // Offset within the file to where the data is located.
    unsigned int mDataOffset = 0;
    
    // Map of asset name to an asset handle.
    // The asset needs to be extracted before it can be used.
    std::unordered_map<std::string, BarnAsset> mAssetMap;
};
