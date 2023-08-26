//
// Clark Kromenaker
//
// Base class for any game asset.
// An asset is any piece of game content - textures, meshes, audio, scripts, etc.
// 
// Usually loaded from the disk, but could be created at runtime as well.
//
#pragma once
#include <cstdint>
#include <string>

// Assets can have an assigned scope, which helps to inform memory management.
enum class AssetScope
{
    Global,     // An asset with Global scope is only unloaded from memory if explicitly requested.
    Scene,      // An asset with Scene scope is unloaded when the scene changes.

    Manual      // An asset with manual scope is not tracked by the system, so the creator of the asset is responsible for its lifetime.
};

class Asset
{
public:
    virtual ~Asset() = default;
    
    const std::string& GetName() const { return mName; }
    std::string GetNameNoExtension() const;

    void SetScope(AssetScope scope) { mScope = scope; }
    AssetScope GetScope() const { return mScope; }
    
protected:
    // You should not be able to create an instance of this class - only subclasses are allowed.
    explicit Asset(const std::string& name, AssetScope scope = AssetScope::Manual);

    // Asset's name, typically including an extension.
    std::string mName;

    // Asset's scope.
    AssetScope mScope = AssetScope::Global;
};
