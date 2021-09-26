#include "TextAsset.h"

#include <cstring>

TextAsset::TextAsset(const std::string& name, char* data, int dataLength) : Asset(name),
    mText(data),
    mTextLength(dataLength)
{

}

TextAsset::~TextAsset()
{
    delete[] mText;
}
