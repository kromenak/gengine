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
	// Get CHARACTERS text file as a raw buffer.
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
		
		// Each entry in a section is some property about the character.
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
	
	// Get FACES text file as a raw buffer.
	unsigned int facesBufferSize = 0;
	char* facesBuffer = Services::GetAssets()->LoadRaw("FACES.TXT", facesBufferSize);
	
	// Pass that along to INI parser, since it is plain text and in INI format.
	IniParser facesParser(facesBuffer, facesBufferSize);
	facesParser.SetMultipleKeyValuePairsPerLine(false); // Stops splitting on commas.
	
	// Read in each section of the FACES file.
	while(facesParser.ReadNextSection(section))
	{
		// Default section contains some values to use if not defined for a particular character.
		if(StringUtil::EqualsIgnoreCase(section.name, "Default"))
		{
			
		}
		else if(StringUtil::EqualsIgnoreCase(section.name, "Eyes"))
		{
			// Eyes section contains eye definitions.
			// But they are all the same...<bitmap> = 4x4, DownSampleOnly
			// No need to read in.
		}
		else
		{
			// If the section name matches a character identifier, this section is the face info for that character!
			auto it = mCharacterConfigs.find(section.name);
			if(it != mCharacterConfigs.end())
			{
				CharacterConfig& config = it->second;
				
				// Each entry is a face property for the character.
				for(auto& entry : section.entries)
				{
					if(StringUtil::EqualsIgnoreCase(entry->key, "Left Eye Name"))
					{
						config.faceConfig.leftEyeTexture = Services::GetAssets()->LoadTexture(entry->value);
					}
					else if(StringUtil::EqualsIgnoreCase(entry->key, "Right Eye Name"))
					{
						config.faceConfig.rightEyeTexture = Services::GetAssets()->LoadTexture(entry->value);
					}
					else if(StringUtil::EqualsIgnoreCase(entry->key, "Left Eye Offset"))
					{
						config.faceConfig.leftEyeOffset = entry->GetValueAsVector2();
					}
					else if(StringUtil::EqualsIgnoreCase(entry->key, "Right Eye Offset"))
					{
						config.faceConfig.rightEyeOffset = entry->GetValueAsVector2();
					}
					else if(StringUtil::EqualsIgnoreCase(entry->key, "Left Eye Bias"))
					{
						config.faceConfig.leftEyeBias = entry->GetValueAsVector2();
					}
					else if(StringUtil::EqualsIgnoreCase(entry->key, "Right Eye Bias"))
					{
						config.faceConfig.rightEyeBias = entry->GetValueAsVector2();
					}
				}
			}
		}
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
	if(location.empty())
	{
		SetCharacterOffstage(name);
	}
	else
	{
		mCharacterLocations[name] = location;
	}
}

std::string CharacterManager::GetCharacterLocation(std::string name) const
{
	auto it = mCharacterLocations.find(name);
	if(it != mCharacterLocations.end())
	{
		return it->second;
	}
	return "";
}

void CharacterManager::SetCharacterOffstage(std::string name)
{
	auto it = mCharacterLocations.find(name);
	if(it != mCharacterLocations.end())
	{
		mCharacterLocations.erase(it);
	}
}

bool CharacterManager::IsCharacterOffstage(std::string name) const
{
	auto it = mCharacterLocations.find(name);
	return it == mCharacterLocations.end();
}
