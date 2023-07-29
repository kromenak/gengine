#include "Sequence.h"

#include <string>

#include "AssetManager.h"
#include "IniParser.h"
#include "StringUtil.h"

void Sequence::Load(char* data, int dataLength)
{
    IniParser parser(data, dataLength);
    parser.SetMultipleKeyValuePairsPerLine(false);
    while(parser.ReadLine())
    {
        while(parser.ReadKeyValuePair())
        {
            const IniKeyValue& keyValuePair = parser.GetKeyValue();
            if(StringUtil::EqualsIgnoreCase(keyValuePair.key, "Sequence Type"))
            {
                //TODO: Set sequence type, assuming "Series" for now.
            }
            else if(StringUtil::EqualsIgnoreCase(keyValuePair.key, "Sprite List"))
            {
                std::vector<std::string> textureNames = StringUtil::Split(keyValuePair.value, ',');
                for(auto& texName : textureNames)
                {
                    mTextures.push_back(gAssetManager.LoadTexture(texName, GetScope()));
                }
            }
        }
    }
}