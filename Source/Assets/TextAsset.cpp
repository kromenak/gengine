#include "TextAsset.h"

TYPEINFO_INIT(TextAsset, Asset, GENERATE_TYPE_ID)
{

}

TextAsset::~TextAsset()
{
    delete[] mText;
}

void TextAsset::Load(uint8_t* data, uint32_t dataLength)
{
    mText = data;
    mTextLength = dataLength;
}
