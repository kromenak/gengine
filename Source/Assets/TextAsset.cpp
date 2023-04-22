#include "TextAsset.h"

#include <cstring>

TextAsset::TextAsset(const std::string& name, AssetScope scope, char* data, int dataLength) : Asset(name, scope),
    mText(data),
    mTextLength(dataLength)
{

}

TextAsset::~TextAsset()
{
    delete[] mText;
}
