//
// TextAsset.h
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
    TextAsset(const std::string& name, char* data, int dataLength);
    ~TextAsset();
    
    char* GetText() { return mText; }
    int GetTextLength() { return mTextLength; }
    
private:
    char* mText = nullptr;
    int mTextLength = 0;
};
