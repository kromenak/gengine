//
// SceneModel.cpp
//
// Clark Kromenaker
//
#include "SceneModel.h"

#include <iostream>

#include "IniParser.h"
#include "Services.h"
#include "Skybox.h"
#include "StringUtil.h"

SceneModel::SceneModel(std::string name, char* data, int dataLength) : Asset(name)
{
    ParseFromData(data, dataLength);
}

std::string SceneModel::GetBSPName()
{
    // If an override is specified, use it!
    if(!mBSPNameOverride.empty())
    {
        return mBSPNameOverride;
    }
    
    // Otherwise, we default to our asset name, with no extension.
	return GetNameNoExtension();
}

void SceneModel::ParseFromData(char *data, int dataLength)
{
    IniParser parser(data, dataLength);
    parser.ParseAll();
    
    // Load any BSP override specified. Otherwise, the name of the SCN asset is used for the BSP too.
    std::vector<IniSection> topSections = parser.GetSections("");
    for(auto& section : topSections)
    {
        for(auto& entry : section.entries)
        {
            if(StringUtil::EqualsIgnoreCase(entry->key, "bsp"))
            {
                mBSPNameOverride = entry->value;
            }
        }
    }
    
    // Load a skybox, if any.
    IniSection skyboxSection = parser.GetSection("SKYBOX");
    if(skyboxSection.entries.size() > 0)
    {
        mSkybox = new Skybox();
    }
    for(auto& entry : skyboxSection.entries)
    {
        if(StringUtil::EqualsIgnoreCase(entry->key, "left"))
        {
			Texture* texture = Services::GetAssets()->LoadTexture(entry->value);
            mSkybox->SetLeftTexture(texture);
        }
        else if(StringUtil::EqualsIgnoreCase(entry->key, "right"))
        {
			Texture* texture = Services::GetAssets()->LoadTexture(entry->value);
            mSkybox->SetRightTexture(texture);
        }
        else if(StringUtil::EqualsIgnoreCase(entry->key, "front"))
        {
			Texture* texture = Services::GetAssets()->LoadTexture(entry->value);
            mSkybox->SetFrontTexture(texture);
        }
        else if(StringUtil::EqualsIgnoreCase(entry->key, "back"))
        {
			Texture* texture = Services::GetAssets()->LoadTexture(entry->value);
            mSkybox->SetBackTexture(texture);
        }
        else if(StringUtil::EqualsIgnoreCase(entry->key, "up"))
        {
			Texture* texture = Services::GetAssets()->LoadTexture(entry->value);
            mSkybox->SetUpTexture(texture);
        }
        else if(StringUtil::EqualsIgnoreCase(entry->key, "down"))
        {
			Texture* texture = Services::GetAssets()->LoadTexture(entry->value);
            mSkybox->SetDownTexture(texture);
        }
    }
}
