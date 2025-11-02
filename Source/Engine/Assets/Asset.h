//
// Clark Kromenaker
//
// Base class for any game asset.
// An asset is any piece of game content - textures, meshes, audio, scripts, etc.
//
// Usually loaded from the disk, but could be created at runtime as well.
//
#pragma once
#include <memory>
#include <string>

#include "TypeInfo.h"

// Assets can have an assigned scope, which helps to inform memory management.
enum class AssetScope
{
    Global,     // An asset with Global scope is only unloaded from memory if explicitly requested.
    Scene,      // An asset with Scene scope is unloaded when the scene changes.

    Manual      // An asset with manual scope is not tracked by the system, so the creator of the asset is responsible for its lifetime.
};

// Holds raw asset data to be passed to an Asset::Load function.
struct AssetData
{
    // A unique_ptr allows the Load function to take ownership of the byte data, if desired.
    // A few assets want to keep the byte buffer in memory, while others just parse it and then want to delete it.
    std::unique_ptr<uint8_t> bytes = nullptr;
    uint32_t length = 0;
};

class Asset
{
    TYPEINFO_BASE(Asset);
public:
    virtual ~Asset() = default;

    const std::string& GetName() const { return mName; }
    std::string GetNameNoExtension() const;

    void SetScope(AssetScope scope) { mScope = scope; }
    AssetScope GetScope() const { return mScope; }

protected:
    // Asset's name, typically including an extension.
    std::string mName;

    // Asset's scope.
    AssetScope mScope = AssetScope::Global;

    // You should not be able to create an instance of this class - only subclasses are allowed.
    explicit Asset(const std::string& name, AssetScope scope);
};