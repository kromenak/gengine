//
// SceneAsset.cpp
//
// Clark Kromenaker
//
#include "SceneAsset.h"

#include <iostream>

#include "IniParser.h"
#include "Services.h"
#include "Skybox.h"
#include "StringUtil.h"

SceneAsset::SceneAsset(std::string name, char* data, int dataLength) : Asset(name)
{
    ParseFromData(data, dataLength);
}

SceneAsset::~SceneAsset()
{
	delete mSkybox;
}

void SceneAsset::ParseFromData(char *data, int dataLength)
{
    IniParser parser(data, dataLength);
    parser.ParseAll();
    
    // Load any BSP override specified. Otherwise, the name of the SCN asset is used for the BSP too.
    std::vector<IniSection> topSections = parser.GetSections("");
    for(auto& section : topSections)
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
	
	// If no BSP name was specified in the SCN file, default to the asset name with no extension.
	if(mBspName.empty())
	{
		mBspName = GetNameNoExtension();
	}
    
    // Load a skybox, if any.
    IniSection skyboxSection = parser.GetSection("SKYBOX");
    if(skyboxSection.lines.size() > 0)
    {
        mSkybox = new Skybox();
    }
	for(auto& line : skyboxSection.lines)
    {
		IniKeyValue& entry = line.entries.front();
        
        if(StringUtil::EqualsIgnoreCase(entry.key, "left"))
        {
			Texture* texture = Services::GetAssets()->LoadSceneTexture(entry.value);
            mSkybox->SetLeftTexture(texture);
        }
        else if(StringUtil::EqualsIgnoreCase(entry.key, "right"))
        {
			Texture* texture = Services::GetAssets()->LoadSceneTexture(entry.value);
            mSkybox->SetRightTexture(texture);
        }
        else if(StringUtil::EqualsIgnoreCase(entry.key, "front"))
        {
			Texture* texture = Services::GetAssets()->LoadSceneTexture(entry.value);
            mSkybox->SetFrontTexture(texture);
        }
        else if(StringUtil::EqualsIgnoreCase(entry.key, "back"))
        {
			Texture* texture = Services::GetAssets()->LoadSceneTexture(entry.value);
            mSkybox->SetBackTexture(texture);
        }
        else if(StringUtil::EqualsIgnoreCase(entry.key, "up"))
        {
			Texture* texture = Services::GetAssets()->LoadSceneTexture(entry.value);
            mSkybox->SetUpTexture(texture);
        }
        else if(StringUtil::EqualsIgnoreCase(entry.key, "down"))
        {
			Texture* texture = Services::GetAssets()->LoadSceneTexture(entry.value);
            mSkybox->SetDownTexture(texture);
        }
    }
}
