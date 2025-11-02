//
// Clark Kromenaker
//
// Sometimes, systems just need to load and read text files.
// This is just a simple wrapper around that text data.
//
#pragma once
#include "Asset.h"

class TextAsset : public Asset
{
    TYPEINFO_SUB(TextAsset, Asset);
public:
    TextAsset(const std::string& name, AssetScope scope) : Asset(name, scope) { }
    ~TextAsset() override;

    void Load(AssetData& data);

    uint8_t* GetText() const { return mText; }
    uint32_t GetTextLength() const { return mTextLength; }

private:
    // The text and text length. Owned by this object, deleted on destruct.
    uint8_t* mText = nullptr;
    uint32_t mTextLength = 0;
};