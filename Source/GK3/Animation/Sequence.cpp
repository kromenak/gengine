#include "Sequence.h"

#include <string>

#include "AssetManager.h"
#include "IniReader.h"
#include "StringUtil.h"
#include "Texture.h"

TYPEINFO_INIT(Sequence, Asset, GENERATE_TYPE_ID)
{
    TYPEINFO_VAR(Sequence, VariableType::Int, mFramesPerSecond);
}

void Sequence::Load(AssetData& data)
{
    IniReader parser(data.bytes.get(), data.length);
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
                    mTextures.push_back(gAssetManager.LoadAsset<Texture>(texName, GetScope()));
                }
            }
        }
    }
}