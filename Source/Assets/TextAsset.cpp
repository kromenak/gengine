#include "TextAsset.h"

#include <cstring>

TextAsset::~TextAsset()
{
    delete[] mText;
}

void TextAsset::Load(char* data, int dataLength)
{
    mText = data;
    mTextLength = dataLength;
}
