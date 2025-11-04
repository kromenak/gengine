//
// Clark Kromenaker
//
// Interface for any asset archive solution.
// An "asset archive" is a collection of assets, usually stored as a binary blob on the disk.
//
#pragma once
#include <cstdint>
#include <functional>
#include <string>

class IAssetArchive
{
public:
    virtual ~IAssetArchive() = default;
    virtual const std::string& GetName() const = 0;
    virtual uint8_t* CreateAssetBuffer(const std::string& assetName, uint32_t& outBufferSize) const = 0;
    virtual void ForEachAsset(const std::function<void(const std::string&)>& callback) const = 0;
};