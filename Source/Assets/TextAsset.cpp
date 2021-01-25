//
// TextAsset.cpp
//
// Clark Kromenaker
//
#include "TextAsset.h"

#include <cstring>

TextAsset::TextAsset(const std::string& name, char* data, int dataLength) : Asset(name)
{
    mText = new char[dataLength];
    memcpy(mText, data, dataLength);
    mTextLength = dataLength;
}

TextAsset::~TextAsset()
{
    delete[] mText;
}
