//
// Clark Kromenaker
//
// In GK3, a "Barn" is a blob of binary data containing assets.
//
// Fun fact: it is called a "Barn" because some assets are named after animals (Sheep, Yak).
// That metaphor didn't extend super far, but you get the idea.
//
#pragma once
#include <mutex>
#include <string>
#include <unordered_map>

#include "BinaryReader.h"
#include "StringUtil.h"

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
    // This is ONLY set if the asset is not actually contained in the current Barn (e.g. it's a "pointer" to another barn file).
    std::string barnFileName;

    // The name of the asset.
    std::string name;

    // Offset of this asset within the Barn file data blob.
    unsigned int offset = 0;

    // If the asset is compressed, and what it's compressed size is.
    CompressionType compressionType = CompressionType::None;
    unsigned int compressedSize = 0;

    // The uncompressed size of the asset.
    // If the asset is not compressed, this size is equal to compressedSize.
    unsigned int uncompressedSize = 0;

    // True if this BarnAsset is just a pointer to another barn file.
    bool IsPointer() const { return !barnFileName.empty(); }
};

class BarnFile
{
public:
    BarnFile(const std::string& filePath);
	
	// Retrieves an asset handle, if it exists in this bundle.
    BarnAsset* GetAsset(const std::string& assetName);
	
	// Extracts an asset into the provided buffer.
    bool Extract(const std::string& assetName, char* buffer, int bufferSize);
    bool Extract(BarnAsset* asset, char* buffer, int bufferSize);
	
	// For debugging, write assets to file.
    bool WriteToFile(const std::string& assetName);
	bool WriteToFile(const std::string& assetName, const std::string outputDir);
	
	// For debugging, write assets to file whose names match a search string.
	void WriteAllToFile(const std::string& search);
	void WriteAllToFile(const std::string& search, const std::string outputDir);
	
	// For debugging, output asset list to cout.
	void OutputAssetList() const;
    
	const std::string& GetName() const { return mName; }
	
private:
	// Identifiers required to verify file type.
    const int kGameIdentifier = 0x21334B47; // GK3!
    const int kBarnIdentifier = 0x6E726142; // Barn
	
	// Identifiers required to identify data section.
    const int kDDirIdentifier = 0x44446972; // DDir
    const int kDataIdentifier = 0x44617461; // Data
    
    // The name of the barn file.
    std::string mName;
    
    // Offset within the file to where the data is located.
    unsigned int mDataOffset = 0;
    
    // Binary reader for extracting data.
    // Extraction may occur on multiple threads at once, so a mutex is required to guard access.
    BinaryReader mReader;
    std::mutex mReaderMutex;
    
    // Map of asset name to an asset handle. Assets must be extracted before being used.
    // Asset names are case-insensitive.
    std::unordered_map_ci<std::string, BarnAsset> mAssetMap;
};
