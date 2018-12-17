//
// ButtonIconManager.cpp
//
// Clark Kromenaker
//
#include "ButtonIconManager.h"

#include "IniParser.h"
#include "Services.h"
#include "StringUtil.h"

float ButtonIcon::GetWidth() const
{
	if(upTexture != nullptr) { return upTexture->GetWidth(); }
	if(downTexture != nullptr) { return downTexture->GetWidth(); }
	if(hoverTexture != nullptr) { return hoverTexture->GetWidth(); }
	if(disableTexture != nullptr) { return disableTexture->GetWidth(); }
	return 0.0f;
}

TYPE_DEF_BASE(ButtonIconManager);

ButtonIconManager::ButtonIconManager()
{
	// Get VERBS text file as a raw buffer.
	unsigned int bufferSize = 0;
	char* buffer = Services::GetAssets()->LoadRaw("VERBS.TXT", bufferSize);
	
	// Pass that along to INI parser, since it is plain text and in INI format.
	IniParser parser(buffer, bufferSize);
	parser.ParseAll();
	
	// Everything is contained within the "VERBS" section.
	// There's only one section in the whole file.
	IniSection section = parser.GetSection("VERBS");
	
	// Each entry is a single button icon declaration.
	// Format is: KEYWORD, up=, down=, hover=, disable=, type
	for(auto& entry : section.entries)
	{
		// This is a required value.
		std::string keyword = entry->key;
		
		// These values will be filled in with remaining entry keys.
		Texture* upTexture = nullptr;
		Texture* downTexture = nullptr;
		Texture* hoverTexture = nullptr;
		Texture* disableTexture = nullptr;
		std::unordered_map<std::string, ButtonIcon>* map = &mVerbsToIcons;
		
		// The remaining values are all optional.
		// If a value isn't present, the above defaults are used.
		IniKeyValue* keyValuePair = entry->next;
		while(keyValuePair != nullptr)
		{
			if(StringUtil::EqualsIgnoreCase(keyValuePair->key, "up"))
			{
				upTexture = Services::GetAssets()->LoadTexture(keyValuePair->value);
			}
			else if(StringUtil::EqualsIgnoreCase(keyValuePair->key, "down"))
			{
				downTexture = Services::GetAssets()->LoadTexture(keyValuePair->value);
			}
			else if(StringUtil::EqualsIgnoreCase(keyValuePair->key, "hover"))
			{
				hoverTexture = Services::GetAssets()->LoadTexture(keyValuePair->value);
			}
			else if(StringUtil::EqualsIgnoreCase(keyValuePair->key, "disable"))
			{
				disableTexture = Services::GetAssets()->LoadTexture(keyValuePair->value);
			}
			else if(StringUtil::EqualsIgnoreCase(keyValuePair->key, "type"))
			{
				if(StringUtil::EqualsIgnoreCase(keyValuePair->value, "inventory"))
				{
					map = &mNounsToIcons;
				}
				else if(StringUtil::EqualsIgnoreCase(keyValuePair->value, "topic"))
				{
					map = &mTopicsToIcons;
				}
			}
			keyValuePair = keyValuePair->next;
		}
		
		// As long as any texture was set, we'll save this one.
		if(upTexture != nullptr || downTexture != nullptr ||
		   hoverTexture != nullptr || disableTexture != nullptr)
		{
			ButtonIcon buttonIcon;
			buttonIcon.upTexture = upTexture;
			buttonIcon.downTexture = downTexture;
			buttonIcon.hoverTexture = hoverTexture;
			buttonIcon.disableTexture = disableTexture;
			
			// Save one of these as the default.
			if(keyword == "QUESTION")
			{
				mDefaultIcon = buttonIcon;
			}
			
			// Insert mapping from keyword to the button icons.
			map->insert({ keyword, buttonIcon });
		}
	}
}

ButtonIcon& ButtonIconManager::GetButtonIconForNoun(std::string noun)
{
	if(mNounsToIcons.find(noun) != mNounsToIcons.end())
	{
		return mNounsToIcons[noun];
	}
	else
	{
		return mDefaultIcon;
	}
}

ButtonIcon& ButtonIconManager::GetButtonIconForVerb(std::string verb)
{
	if(mVerbsToIcons.find(verb) != mVerbsToIcons.end())
	{
		return mVerbsToIcons[verb];
	}
	else
	{
		return mDefaultIcon;
	}
}

ButtonIcon& ButtonIconManager::GetButtonIconForTopic(std::string topic)
{
	if(mTopicsToIcons.find(topic) != mTopicsToIcons.end())
	{
		return mTopicsToIcons[topic];
	}
	else
	{
		return mDefaultIcon;
	}
}
