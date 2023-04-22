#include "Sequence.h"

#include <string>

#include "IniParser.h"
#include "Services.h"
#include "StringUtil.h"

Sequence::Sequence(const std::string& name, AssetScope scope, char* data, int dataLength) : Asset(name, scope)
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
                for(auto& name : textureNames)
                {
                    mTextures.push_back(Services::GetAssets()->LoadTexture(name, GetScope()));
                }
            }
        }
    }
}