#include "TextAsset.h"

TYPEINFO_INIT(TextAsset, Asset, GENERATE_TYPE_ID)
{

}

TextAsset::~TextAsset()
{
    delete[] mText;
}

void TextAsset::Load(AssetData& data)
{
    // Take ownership of the byte buffer.
    mText = data.bytes.release();
    mTextLength = data.length;
}