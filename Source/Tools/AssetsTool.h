//
// Clark Kromenaker
//
// A tool that shows loaded assets and details about the assets themselves.
//
#pragma once
#include <string>

class Asset;

class AssetsTool
{
public:
    void Render(bool& toolActive);

private:
    // The asset that is currently selected.
    Asset* mSelectedAsset = nullptr;

    template<typename T> void AddAssetList(const std::string& id = "");
};