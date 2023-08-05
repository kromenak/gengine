//
// Clark Kromenaker
//
// In GK3, a "Barn" is a blob of binary data containing assets.
//
// Fun fact: it is called a "Barn" because some assets are named after animals (Sheep, Yak).
// That metaphor didn't extend super far, but you get the idea.
//
#pragma once
#include <cstdint>
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

struct BarnAsset
{
    // Name of barn file containing this asset.
    // This is ONLY non-null if the asset is contained in some other Barn (e.g. it's a "pointer" to another Barn file).
    std::string* barnFileName = nullptr;

    // The name of the asset.
    std::string name;

    // Offset of this asset within the Barn file data blob.
    uint32_t offset = 0;

    // The size of this asset's data in the Barn file data blob.
    uint32_t size = 0;

    // Compression type for this asset.
    // If set, the asset needs to be decompressed to be usable.
    CompressionType compressionType = CompressionType::None;
    
    // True if this BarnAsset is just a pointer to another barn file.
    bool IsPointer() const { return barnFileName != nullptr; }
};

class BarnFile
{
public:
    BarnFile(const std::string& filePath);

	// Retrieves an asset handle, if it exists in this bundle.
    BarnAsset* GetAsset(const std::string& assetName);

    // Creates a buffer containing the desired asset. Caller owns the returned buffer.
    uint8_t* CreateAssetBuffer(const std::string& assetName, uint32_t& outBufferSize);

	// For debugging, write assets to file.
    bool WriteToFile(const std::string& assetName);
	bool WriteToFile(const std::string& assetName, const std::string& outputDir);
	
	// For debugging, write assets to file whose names match a search string.
	void WriteAllToFile(const std::string& search);
	void WriteAllToFile(const std::string& search, const std::string& outputDir);
	
	// For debugging, output asset list to cout.
	void OutputAssetList() const;
    
	const std::string& GetName() const { return mName; }
	
private:
	// Identifiers required to verify file type.
    const uint32_t kGameIdentifier = 0x21334B47; // GK3!
    const uint32_t kBarnIdentifier = 0x6E726142; // Barn
	
	// Identifiers required to identify data section.
    const uint32_t kDDirIdentifier = 0x44446972; // DDir
    const uint32_t kDataIdentifier = 0x44617461; // Data
    
    // The name of the barn file.
    std::string mName;
    
    // Offset within the file to where the data is located.
    uint32_t mDataOffset = 0;
    
    // Binary reader for extracting data.
    // Extraction may occur on multiple threads at once, so a mutex is required to guard access.
    BinaryReader mReader;
    std::mutex mReaderMutex;

    // If *this* Barn contains pointers to *other* Barns, this contains the names of those other Barns.
    // Individual assets that are pointers will point to these elements.
    std::vector<std::string> mReferencedBarns;
    
    // Map of asset name to an asset handle. Assets must be extracted before being used.
    // Asset names are case-insensitive.
    std::string_map_ci<BarnAsset> mAssetMap;
};
