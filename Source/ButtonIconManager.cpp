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
	
	// Each line is a single button icon declaration.
	// Format is: KEYWORD, up=, down=, hover=, disable=, type
	for(auto& line : section.lines)
	{
		IniKeyValue& entry = line.entries.front();
		
		// This is a required value.
		std::string keyword = StringUtil::ToLowerCopy(entry.key);
		
		// These values will be filled in with remaining entry keys.
		Texture* upTexture = nullptr;
		Texture* downTexture = nullptr;
		Texture* hoverTexture = nullptr;
		Texture* disableTexture = nullptr;
		
		// By default, the type of each button is a verb.
		// However, if the keyword "inventory" or "topic" are used, the button is put in those maps instead.
		// This is why I bother using a pointer to a map here!
		std::unordered_map<std::string, ButtonIcon>* map = &mVerbsToIcons;
		
		// The remaining values are all optional.
		// If a value isn't present, the above defaults are used.
		for(int i = 1; i < line.entries.size(); ++i)
		{
			IniKeyValue& keyValuePair = line.entries[i];
			if(StringUtil::EqualsIgnoreCase(keyValuePair.key, "up"))
			{
				upTexture = Services::GetAssets()->LoadTexture(keyValuePair.value);
			}
			else if(StringUtil::EqualsIgnoreCase(keyValuePair.key, "down"))
			{
				downTexture = Services::GetAssets()->LoadTexture(keyValuePair.value);
			}
			else if(StringUtil::EqualsIgnoreCase(keyValuePair.key, "hover"))
			{
				hoverTexture = Services::GetAssets()->LoadTexture(keyValuePair.value);
			}
			else if(StringUtil::EqualsIgnoreCase(keyValuePair.key, "disable"))
			{
				disableTexture = Services::GetAssets()->LoadTexture(keyValuePair.value);
			}
			else if(StringUtil::EqualsIgnoreCase(keyValuePair.key, "type"))
			{
				if(StringUtil::EqualsIgnoreCase(keyValuePair.value, "inventory"))
				{
					map = &mNounsToIcons;
				}
				else if(StringUtil::EqualsIgnoreCase(keyValuePair.value, "topic") ||
						StringUtil::EqualsIgnoreCase(keyValuePair.value, "chat"))
				{
					map = &mTopicsToIcons;
				}
			}
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
			
			// Save one of these as the default icon.
			// "Question mark" seems like as good as any!
			if(StringUtil::EqualsIgnoreCase(keyword, "QUESTION"))
			{
				mDefaultIcon = buttonIcon;
			}
			
			// Insert mapping from keyword to the button icons.
			map->insert({ keyword, buttonIcon });
		}
	}
	delete[] buffer;
}

ButtonIcon& ButtonIconManager::GetButtonIconForNoun(const std::string& noun)
{
	auto it = mNounsToIcons.find(StringUtil::ToLowerCopy(noun));
	if(it != mNounsToIcons.end())
	{
		return it->second;
	}
	else
	{
		return mDefaultIcon;
	}
}

ButtonIcon& ButtonIconManager::GetButtonIconForVerb(const std::string& verb)
{
	auto it = mVerbsToIcons.find(StringUtil::ToLowerCopy(verb));
	if(it != mVerbsToIcons.end())
	{
		return it->second;
	}
	else
	{
		return mDefaultIcon;
	}
}

ButtonIcon& ButtonIconManager::GetButtonIconForTopic(const std::string& topic)
{
	auto it = mTopicsToIcons.find(StringUtil::ToLowerCopy(topic));
	if(it != mTopicsToIcons.end())
	{
		return it->second;
	}
	else
	{
		return mDefaultIcon;
	}
}

bool ButtonIconManager::IsVerb(const std::string& word)
{
	return mVerbsToIcons.find(StringUtil::ToLowerCopy(word)) != mVerbsToIcons.end();
}

bool ButtonIconManager::IsInventoryItem(const std::string& word)
{
	return mNounsToIcons.find(StringUtil::ToLowerCopy(word)) != mNounsToIcons.end();
}

bool ButtonIconManager::IsTopic(const std::string& word)
{
	return mTopicsToIcons.find(StringUtil::ToLowerCopy(word)) != mTopicsToIcons.end();
}
