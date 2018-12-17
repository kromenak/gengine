//
// CharacterManager.cpp
//
// Clark Kromenaker
//
#include "CharacterManager.h"

#include "IniParser.h"
#include "Services.h"
#include "StringUtil.h"

TYPE_DEF_BASE(CharacterManager);

CharacterManager::CharacterManager()
{
	// Get VERBS text file as a raw buffer.
	unsigned int bufferSize = 0;
	char* buffer = Services::GetAssets()->LoadRaw("CHARACTERS.TXT", bufferSize);
	
	// Pass that along to INI parser, since it is plain text and in INI format.
	IniParser parser(buffer, bufferSize);
	
	// Read one section at a time.
	// Each section correlates to one character.
	// The section name is the character's three-letter code (GAB, ABE, GRA, etc).
	IniSection section;
	while(parser.ReadNextSection(section))
	{
		CharacterConfig config;
		config.identifier = section.name;
		
		for(auto& entry : section.entries)
		{
			if(StringUtil::EqualsIgnoreCase(entry->key, "WalkerHeight"))
			{
				config.walkerHeight = entry->GetValueAsFloat();
			}
			else if(StringUtil::EqualsIgnoreCase(entry->key, "StartAnim"))
			{
				config.walkStartAnim = Services::GetAssets()->LoadAnimation(entry->value);
			}
			else if(StringUtil::EqualsIgnoreCase(entry->key, "ContAnim"))
			{
				config.walkLoopAnim = Services::GetAssets()->LoadAnimation(entry->value);
			}
			else if(StringUtil::EqualsIgnoreCase(entry->key, "StopAnim"))
			{
				config.walkStopAnim = Services::GetAssets()->LoadAnimation(entry->value);
			}
			else if(StringUtil::EqualsIgnoreCase(entry->key, "StartTurnRightAnim"))
			{
				config.walkStartTurnRightAnim = Services::GetAssets()->LoadAnimation(entry->value);
			}
			else if(StringUtil::EqualsIgnoreCase(entry->key, "StartTurnLeftAnim"))
			{
				config.walkStartTurnLeftAnim = Services::GetAssets()->LoadAnimation(entry->value);
			}
		}
		
		// Key each config by its identifier.
		mCharacterConfigs[config.identifier] = config;
	}
}

CharacterConfig& CharacterManager::GetCharacterConfig(std::string identifier)
{
	auto it = mCharacterConfigs.find(identifier);
	if(it != mCharacterConfigs.end())
	{
		return mCharacterConfigs[identifier];
	}
	return mDefaultCharacterConfig;
}

void CharacterManager::SetCharacterLocation(std::string name, std::string location)
{
	mCharacterLocations[name] = location;
}

std::string CharacterManager::GetCharacterLocation(std::string name) const
{
	auto it = mCharacterLocations.find(name);
	if(it != mCharacterLocations.end())
	{
		return it->second;
	}
	return std::string();
}

bool CharacterManager::IsCharacterOffstage(std::string name) const
{
	auto it = mCharacterLocations.find(name);
	return it == mCharacterLocations.end();
}
