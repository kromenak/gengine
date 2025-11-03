#include "AssetManager.h"

#include <algorithm> // std::sort
#include <cstring>
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

std::string AssetManager::GetAssetPath(const std::string& fileName) const
{
    // Iterate each search path and see if a file with this filename exists at that path.
    std::string assetPath;
    for(auto& searchPath : mSearchPaths)
    {
        if(Path::FindFullPath(fileName, searchPath, assetPath))
        {
            return assetPath;
        }
    }
    return std::string();
}

std::string AssetManager::GetAssetPath(const std::string& fileName, std::initializer_list<std::string> extensions) const
{
    // If already has an extension, just use the normal path find function.
    if(Path::HasExtension(fileName))
    {
        return GetAssetPath(fileName);
    }

    // Otherwise, we have a filename, but multiple valid extensions.
    // A good example is a movie file. The file might be called "intro", but the extension could be "avi" or "bik".
    // Iterate possible extensions and try to find a valid asset path.
    for(const std::string& extension : extensions)
    {
        std::string assetPath = GetAssetPath(fileName + "." + extension);
        if(!assetPath.empty())
        {
            return assetPath;
        }
    }
    return std::string();
}

bool AssetManager::LoadAssetArchive(const std::string& archiveName, int searchOrder)
{
    // Find the archive on the disk, or fail.
    std::string archivePath = GetAssetPath(archiveName);
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

void AssetManager::ExtractAsset(const std::string& assetName, const std::string& outputDirectory) const
{
    // Find archive containing this asset and extract it.
    IAssetArchive* archive = GetArchiveContainingAsset(assetName);
    if(archive != nullptr && archive->ExtractAsset(assetName, outputDirectory))
    {
        printf("Extracted asset %s to %s\n", assetName.c_str(), outputDirectory.c_str());
    }
    else
    {
        printf("Could not extract asset %s\n", assetName.c_str());
    }
}

void AssetManager::ExtractAssets(const std::string& search, const std::string& outputDirectory) const
{
    // Pass the buck to all loaded barn files.
    uint32_t extractCount = 0;
    for(auto& entry : mArchives)
    {
        extractCount += entry.archive->ExtractAssets(search, outputDirectory);
    }
    printf("Extracted %u assets matching search string %s.\n", extractCount, search.c_str());
}

void AssetManager::UnloadAssets(AssetScope scope)
{
    for(auto& entry : IAssetCache::sAssetCachesByType)
    {
        for(auto& assetCache : entry.second)
        {
            assetCache->UnloadAssets(scope);
        }
    }
}

IAssetArchive* AssetManager::GetArchiveContainingAsset(const std::string& assetName) const
{
    // The archives array is already sorted by priority, so just return the first one that contains a matching asset.
    for(auto& entry : mArchives)
    {
        if(entry.archive->ContainsAsset(assetName))
        {
            return entry.archive;
        }
    }
    return nullptr;
}

std::string AssetManager::SanitizeAssetName(const std::string& assetName, const std::string& expectedExtension)
{
    // If a three-letter extension already exists, accept it and assume the caller knows what they're doing.
    // Only for 3-letter extensions! GK3 actually includes periods in a few asset names, but never with a three letter ending.
    int lastIndex = assetName.size() - 1;
    if(lastIndex > 3 && assetName[lastIndex - 3] == '.')
    {
        return assetName;
    }

    // No three-letter extension, add the expected extension if missing.
    if(!Path::HasExtension(assetName, expectedExtension))
    {
        return assetName + expectedExtension;
    }
    return assetName;
}

uint8_t* AssetManager::CreateAssetBuffer(const std::string& assetName, uint32_t& outBufferSize) const
{
    // First, see if the asset exists at any search path. If so, we load the asset directly from file.
    // Loose files take precedence over archived assets.
    std::string assetPath = GetAssetPath(assetName);
    if(!assetPath.empty())
    {
        return File::ReadIntoBuffer(assetPath, outBufferSize);
    }

    // If no loose file to load, we'll get the asset from am asset archive.
    IAssetArchive* archive = GetArchiveContainingAsset(assetName);
    if(archive != nullptr)
    {
        return archive->CreateAssetBuffer(assetName, outBufferSize);
    }

    // Couldn't find this asset!
    return nullptr;
}