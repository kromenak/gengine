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
public:
    TextAsset(const std::string& name, AssetScope scope) : Asset(name, scope) { }
    ~TextAsset();

    void Load(char* data, int dataLength);
    
    char* GetText() { return mText; }
    int GetTextLength() { return mTextLength; }
    
private:
    // The text and text length. Owned by this object, deleted on destruct.
    char* mText = nullptr;
    int mTextLength = 0;
};
