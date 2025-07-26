#include "SceneAsset.h"

#include <iostream>

#include "AssetManager.h"
#include "IniParser.h"
#include "Skybox.h"
#include "StringUtil.h"

void SceneAsset::FixGK3SkyboxTextures(SkyboxTextures& textures)
{
    // GK3 puts the front texture in the -Z direction, and the back texture in the +Z direction.
    // I have no idea why this is the case, but even in the original game, this is true.
    std::swap(textures.named.back, textures.named.front);

    // GK3 up/down textures are rotated counter-clockwise 90 degrees.
    // Not sure why this choice was made, but it's required for skybox seams to match up.
    // Note that we can ONLY do this operation here if the textures have SCENE scope.
    // If not, then re-entering the scene would rotate the textures again, giving wrong results.
    if(textures.named.bottom != nullptr && textures.named.bottom->GetScope() == AssetScope::Scene)
    {
        textures.named.bottom->RotateCounterclockwise();
    }
    if(textures.named.top != nullptr && textures.named.top->GetScope() == AssetScope::Scene)
    {
        textures.named.top->RotateCounterclockwise();
    }
}

TYPEINFO_INIT(SceneAsset, Asset, GENERATE_TYPE_ID)
{
    TYPEINFO_VAR(SceneAsset, VariableType::String, mBspName);
}

SceneAsset::~SceneAsset()
{
    delete mSkybox;
}

void SceneAsset::Load(uint8_t* data, uint32_t dataLength)
{
    ParseFromData(data, dataLength);
}

void SceneAsset::ParseFromData(uint8_t* data, uint32_t dataLength)
{
    IniParser parser(data, dataLength);
    parser.SetMultipleKeyValuePairsPerLine(false);

    IniSection section;
    while(parser.ReadNextSection(section))
    {
        if(section.name.empty())
        {
            for(auto& line : section.lines)
            {
                IniKeyValue& entry = line.entries.front();
                if(StringUtil::EqualsIgnoreCase(entry.key, "bsp"))
                {
                    mBspName = entry.value;
                }
            }
        }
        else if(StringUtil::EqualsIgnoreCase(section.name, "Skybox"))
        {
            if(!section.lines.empty())
            {
                SkyboxTextures skyboxTextures;
                float azimuth = 0.0f;
                for(auto& line : section.lines)
                {
                    IniKeyValue& entry = line.entries.front();
                    if(StringUtil::EqualsIgnoreCase(entry.key, "left"))
                    {
                        skyboxTextures.named.left = gAssetManager.LoadSceneTexture(entry.value, GetScope());
                    }
                    else if(StringUtil::EqualsIgnoreCase(entry.key, "right"))
                    {
                        skyboxTextures.named.right = gAssetManager.LoadSceneTexture(entry.value, GetScope());
                    }
                    else if(StringUtil::EqualsIgnoreCase(entry.key, "back"))
                    {
                        skyboxTextures.named.back = gAssetManager.LoadSceneTexture(entry.value, GetScope());
                    }
                    else if(StringUtil::EqualsIgnoreCase(entry.key, "front"))
                    {
                        skyboxTextures.named.front = gAssetManager.LoadSceneTexture(entry.value, GetScope());
                    }
                    else if(StringUtil::EqualsIgnoreCase(entry.key, "down"))
                    {
                        skyboxTextures.named.bottom = gAssetManager.LoadSceneTexture(entry.value, GetScope());
                    }
                    else if(StringUtil::EqualsIgnoreCase(entry.key, "up"))
                    {
                        skyboxTextures.named.top = gAssetManager.LoadSceneTexture(entry.value, GetScope());
                    }
                    else if(StringUtil::EqualsIgnoreCase(entry.key, "azimuth"))
                    {
                        azimuth = Math::ToRadians(entry.GetValueAsFloat());
                    }
                }

                // GK3 does some non-standard stuff with the skybox textures.
                FixGK3SkyboxTextures(skyboxTextures);

                // Create the skybox.
                mSkybox = new Skybox(skyboxTextures);
                mSkybox->SetAzimuth(azimuth);
            }
        }
        else if(StringUtil::EqualsIgnoreCase(section.name, "Lights") || StringUtil::EqualsIgnoreCase(section.name, "Models"))
        {
            // We are not doing anything with these sections right now - except ignoring them.
        }
        else
        {
            /*
            // Any other section MUST BE (as far as I know) a light definition.
            mLights.emplace_back();
            mLights.back().name = section.name;

            for(auto& line : section.lines)
            {
                IniKeyValue& entry = line.entries.front();
                if(StringUtil::EqualsIgnoreCase(entry.key, "Type"))
                {
                    mLights.back().type = entry.GetValueAsInt();
                }
                else if(StringUtil::EqualsIgnoreCase(entry.key, "Position"))
                {
                    mLights.back().position = entry.GetValueAsVector3();
                }
                else if(StringUtil::EqualsIgnoreCase(entry.key, "Direction"))
                {
                    mLights.back().direction = entry.GetValueAsVector3();
                }
                else if(StringUtil::EqualsIgnoreCase(entry.key, "Color"))
                {
                    // Colors in SCN are floating point.
                    // Since we don't yet have support for that, just read in as floats and convert to Color32.
                    Vector3 color = entry.GetValueAsVector3();
                    mLights.back().color = Color32(static_cast<int>(color.x * 255),
                                                   static_cast<int>(color.y * 255),
                                                   static_cast<int>(color.z * 255));
                }
                else if(StringUtil::EqualsIgnoreCase(entry.key, "Hotspot"))
                {
                    mLights.back().hotspot = entry.GetValueAsFloat();
                }
                else if(StringUtil::EqualsIgnoreCase(entry.key, "Falloff"))
                {
                    mLights.back().falloff = entry.GetValueAsFloat();
                }
                else if(StringUtil::EqualsIgnoreCase(entry.key, "AttenStart"))
                {
                    mLights.back().attenuationStart = entry.GetValueAsFloat();
                }
                else if(StringUtil::EqualsIgnoreCase(entry.key, "AttenEnd"))
                {
                    mLights.back().attenuationEnd = entry.GetValueAsFloat();
                }
                else if(StringUtil::EqualsIgnoreCase(entry.key, "UseAtten"))
                {
                    mLights.back().useAttenuation = entry.GetValueAsInt() != 0;
                }
                else if(StringUtil::EqualsIgnoreCase(entry.key, "CastShadows"))
                {
                    mLights.back().castShadows = entry.GetValueAsInt() != 0;
                }
                else if(StringUtil::EqualsIgnoreCase(entry.key, "Overshoot"))
                {
                    mLights.back().overshoot = entry.GetValueAsInt() != 0;
                }
                else if(StringUtil::EqualsIgnoreCase(entry.key, "Intensity"))
                {
                    mLights.back().intensity = entry.GetValueAsFloat();
                }
                else if(StringUtil::EqualsIgnoreCase(entry.key, "Radius"))
                {
                    mLights.back().radius = entry.GetValueAsFloat();
                }
                else if(StringUtil::EqualsIgnoreCase(entry.key, "DecayType"))
                {
                    mLights.back().decayType = entry.GetValueAsInt();
                }
            }
            */
        }
    }

    // If no BSP name was specified in the SCN file, default to the asset name with no extension.
    if(mBspName.empty())
    {
        mBspName = GetNameNoExtension();
    }
}
