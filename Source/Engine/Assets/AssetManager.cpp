#include "AssetManager.h"

#include <algorithm> // std::sort
#include <cstring>
#include <fstream>
#include <string>

#include "BarnFile.h"
#include "FileSystem.h"
#include "StringUtil.h"

AssetManager gAssetManager;

void AssetManager::Shutdown()
{
    // Unload all assets.
    UnloadAssets(AssetScope::Global);

    // Clear all loaded asset archives.
    for(AssetArchive& archive : mArchives)
    {
        delete archive.archive;
    }
    mArchives.clear();
}

void AssetManager::AddSearchPath(const std::string& searchPath)
{
    auto it = std::find(mSearchPaths.begin(), mSearchPaths.end(), searchPath);
    if(it == mSearchPaths.end())
    {
        mSearchPaths.push_back(searchPath);
    }
}

void AssetManager::RemoveSearchPath(const std::string& searchPath)
{
    auto it = std::find(mSearchPaths.begin(), mSearchPaths.end(), searchPath);
    if(it != mSearchPaths.end())
    {
        mSearchPaths.erase(it);
    }
}

std::string AssetManager::FindLooseFilePath(const std::string& fileName) const
{
    // Iterate each search path and see if a file with this filename exists at that path.
    // Search paths are ordered, so higher priority search paths will be checked first (allowing asset overrides).
    std::string assetPath;
    for(auto& searchPath : mSearchPaths)
    {
        if(Path::FindFullPath(fileName, searchPath, assetPath))
        {
            return assetPath;
        }
    }

    // Couldn't find a file with this name at any search path, so just return empty string for failure.
    return std::string();
}

std::string AssetManager::FindLooseFilePath(const std::string& fileName, std::initializer_list<std::string> extensions) const
{
    // If this file already seems to have a valid extension, just try to find its full path.
    if(mAssetNameResolver.HasValidExtension(fileName))
    {
        std::string filePath = FindLooseFilePath(fileName);
        if(!filePath.empty())
        {
            return filePath;
        }
    }

    // Otherwise, try to append the provided extensions and see if any results in a valid file path.
    // A good example of this scenario is movie files. The file might be called "intro", but the extension could be "avi" or "bik".
    std::string assetPath;
    for(const std::string& extension : extensions)
    {
        if(extension.empty()) { continue; }

        // Handle including or not including leading period on extension.
        if(extension.front() == '.')
        {
            assetPath = FindLooseFilePath(fileName + extension);
        }
        else
        {
            assetPath = FindLooseFilePath(fileName + '.' + extension);
        }
        if(!assetPath.empty())
        {
            return assetPath;
        }
    }

    // Can't find a file path for this file at any search path with any extension - return empty string for failure.
    return std::string();
}

bool AssetManager::LoadAssetArchive(const std::string& archiveName, int searchOrder)
{
    // Find the archive on the disk, or fail.
    std::string archivePath = FindLooseFilePath(archiveName);
    if(archivePath.empty())
    {
        return false;
    }

    // Add to list of archives.
    mArchives.emplace_back();
    mArchives.back().archive = new BarnFile(archivePath);
    mArchives.back().searchOrder = searchOrder;

    // Sort archive list based on search order.
    std::sort(mArchives.begin(), mArchives.end(), [](const AssetArchive& a, const AssetArchive& b){
        return a.searchOrder < b.searchOrder;
    });
    return true;
}

void AssetManager::SetAssetExtractor(const std::string& extension, const std::function<bool(AssetExtractData&)>& extractorFunction)
{
    if(extension.empty())
    {
        printf("Empty extension passed to SetAssetExtractor.");
        return;
    }

    // Add the extractor function to the map. Make sure the extension key has a leading dot, for consistency.
    if(extension.front() != '.')
    {
        mAssetExtractorsByExtension["." + extension] = extractorFunction;
    }
    else
    {
        mAssetExtractorsByExtension[extension] = extractorFunction;
    }
}

bool AssetManager::ExtractAsset(const std::string& assetName, const std::string& outputDirectory) const
{
    // Find the first archive that has this asset and extract it.
    // Since archives are ordered, the asset will be extracted from the highest priority archive it exists in.
    for(auto& entry : mArchives)
    {
        if(ExtractAsset(entry.archive, assetName, outputDirectory))
        {
            printf("Extracted asset %s from archive %s to %s\n", assetName.c_str(), entry.archive->GetName().c_str(), outputDirectory.c_str());
            return true;
        }
    }

    printf("Could not extract asset %s\n", assetName.c_str());
    return false;
}

void AssetManager::ExtractAssets(const std::string& search, const std::string& outputDirectory)
{
    // Iterate all archives and extract all assets that contain the provided search string.
    uint32_t extractCount = 0;
    for(auto& entry : mArchives)
    {
        entry.archive->ForEachAsset([this, search, outputDirectory, entry, &extractCount](const std::string& assetName) {
            if(assetName.find(search) != std::string::npos)
            {
                if(ExtractAsset(entry.archive, assetName, outputDirectory))
                {
                    ++extractCount;
                }
            }
        });
    }
    printf("Extracted %u assets matching search string %s.\n", extractCount, search.c_str());
}

void AssetManager::UnloadAssets(AssetScope scope)
{
    // Iterate all asset caches and tell them to unload assets at the given scope.
    for(auto& entry : IAssetCache::sAssetCachesByType)
    {
        for(IAssetCache* assetCache : entry.second)
        {
            assetCache->UnloadAssets(scope);
        }
    }
}

bool AssetManager::ExtractAsset(IAssetArchive* archive, const std::string& assetName,  const std::string& outputDirectory) const
{
    // Must have an archive to extract from.
    if(archive == nullptr) { return false; }

    // Create extract data struct and populate it.
    AssetExtractData extractData;
    extractData.assetName = assetName;

    // Get the raw bytes for the asset to be extracted.
    extractData.assetData.bytes.reset(archive->CreateAssetBuffer(assetName, extractData.assetData.length));
    if(extractData.assetData.bytes == nullptr)
    {
        return false;
    }

    // Decide on an output path. Try to use what's provided, but fall back on working directory worst case.
    if(!outputDirectory.empty() && Directory::CreateAll(outputDirectory))
    {
        extractData.outputPath = Path::Combine({ outputDirectory, assetName });
    }
    else
    {
        extractData.outputPath = assetName;
    }

    // Attempt to extract the asset using a registered asset extractor.
    bool extractSucceeded = false;
    auto it = mAssetExtractorsByExtension.find(Path::GetExtension(assetName, true));
    if(it != mAssetExtractorsByExtension.end())
    {
        extractSucceeded = it->second(extractData);
    }

    // If there is no asset extractor, or if the asset extractor doesn't succeed, fall back on writing the raw bytes.
    // This works for a lot of assets - only a few need custom processing.
    if(!extractSucceeded)
    {
        std::ofstream fileStream(extractData.outputPath, std::istream::out | std::istream::binary);
        if(fileStream.good())
        {
            fileStream.write(reinterpret_cast<char*>(extractData.assetData.bytes.get()), extractData.assetData.length);
            fileStream.close();
            extractSucceeded = true;
        }
    }
    return extractSucceeded;
}

uint8_t* AssetManager::CreateAssetBuffer(const std::string& assetName, uint32_t& outBufferSize) const
{
    // First, see if the asset exists at any search path. If so, we load the asset directly from file.
    // Loose files take precedence over archived assets.
    std::string assetPath = FindLooseFilePath(assetName);
    if(!assetPath.empty())
    {
        return File::ReadIntoBuffer(assetPath, outBufferSize);
    }

    // If no loose file to load, we'll get the asset from an asset archive.
    for(auto& entry : mArchives)
    {
        uint8_t* buffer = entry.archive->CreateAssetBuffer(assetName, outBufferSize);
        if(buffer != nullptr)
        {
            return buffer;
        }
    }

    // Couldn't find this asset!
    return nullptr;
}