#include "VerbManager.h"

#include "AssetManager.h"
#include "IniParser.h"
#include "TextAsset.h"
#include "Texture.h"

float VerbIcon::GetWidth() const
{
    uint32_t width = 0;
	if(upTexture != nullptr) { width = upTexture->GetWidth(); }
	else if(downTexture != nullptr) { width = downTexture->GetWidth(); }
	else if(hoverTexture != nullptr) { width = hoverTexture->GetWidth(); }
    else if(disableTexture != nullptr) { width = disableTexture->GetWidth(); }
    return static_cast<float>(width);
}

VerbManager gVerbManager;

void VerbManager::Init()
{
	// Get VERBS text file.
	TextAsset* text = gAssetManager.LoadText("VERBS.TXT", AssetScope::Manual);
	
	// Pass that along to INI parser, since it is plain text and in INI format.
	IniParser parser(text->GetText(), text->GetTextLength());
	parser.ParseAll();
	
	// Everything is contained within the "VERBS" section.
	// There's only one section in the whole file.
	IniSection section = parser.GetSection("VERBS");
	
	// Each line is a single button icon declaration.
	// Format is: KEYWORD, up=, down=, hover=, disable=, type
    for(auto& line : section.lines)
    {
        IniKeyValue& entry = line.entries.front();

        // These values will be filled in with remaining entry keys.
        Texture* upTexture = nullptr;
        Texture* downTexture = nullptr;
        Texture* hoverTexture = nullptr;
        Texture* disableTexture = nullptr;
        Cursor* cursor = nullptr;

        // By default, the type of each button is a verb.
        // However, if the keyword "inventory" or "topic" are used, the button is put in those maps instead.
        // This is why I bother using a pointer to a map here!
        std::string_map_ci<VerbIcon>* map = &mVerbs;

        // The remaining values are all optional.
        // If a value isn't present, the above defaults are used.
        for(size_t i = 1; i < line.entries.size(); ++i)
        {
            IniKeyValue& keyValuePair = line.entries[i];
            if(StringUtil::EqualsIgnoreCase(keyValuePair.key, "up"))
            {
                upTexture = gAssetManager.LoadTextureAsync(keyValuePair.value);
            }
            else if(StringUtil::EqualsIgnoreCase(keyValuePair.key, "down"))
            {
                downTexture = gAssetManager.LoadTextureAsync(keyValuePair.value);
            }
            else if(StringUtil::EqualsIgnoreCase(keyValuePair.key, "hover"))
            {
                hoverTexture = gAssetManager.LoadTextureAsync(keyValuePair.value);
            }
            else if(StringUtil::EqualsIgnoreCase(keyValuePair.key, "disable"))
            {
                disableTexture = gAssetManager.LoadTextureAsync(keyValuePair.value);
            }
            else if(StringUtil::EqualsIgnoreCase(keyValuePair.key, "cursor"))
            {
                cursor = gAssetManager.LoadCursorAsync(keyValuePair.value);
            }
            else if(StringUtil::EqualsIgnoreCase(keyValuePair.key, "type"))
            {
                if(StringUtil::EqualsIgnoreCase(keyValuePair.value, "inventory"))
                {
                    map = &mInventoryItems;
                }
                else if(StringUtil::EqualsIgnoreCase(keyValuePair.value, "topic"))
                        //StringUtil::EqualsIgnoreCase(keyValuePair.value, "chat"))
                {
                    map = &mTopics;
                }
            }
        }

        // As long as any texture was set, we'll save this one.
        if(upTexture != nullptr || downTexture != nullptr ||
           hoverTexture != nullptr || disableTexture != nullptr || cursor != nullptr)
        {
            VerbIcon verbIcon;
            verbIcon.upTexture = upTexture;
            verbIcon.downTexture = downTexture;
            verbIcon.hoverTexture = hoverTexture;
            verbIcon.disableTexture = disableTexture;
            verbIcon.cursor = cursor;

            // Save one of these as the default icon.
            // "Question mark" seems like as good as any!
            if(StringUtil::EqualsIgnoreCase(entry.key, "QUESTION"))
            {
                mDefaultIcon = verbIcon;
            }

            // Insert mapping from keyword to the button icons.
            map->insert({ entry.key, verbIcon });
        }
    }

    // Done with this asset.
    delete text;
}

VerbIcon& VerbManager::GetInventoryIcon(const std::string& noun)
{
	auto it = mInventoryItems.find(noun);
	if(it != mInventoryItems.end())
	{
		return it->second;
	}
	else
	{
		return mDefaultIcon;
	}
}

VerbIcon& VerbManager::GetVerbIcon(const std::string& verb)
{
	auto it = mVerbs.find(verb);
	if(it != mVerbs.end())
	{
		return it->second;
	}
	else
	{
		return mDefaultIcon;
	}
}

VerbIcon& VerbManager::GetTopicIcon(const std::string& topic)
{
	auto it = mTopics.find(topic);
	if(it != mTopics.end())
	{
		return it->second;
	}
	else
	{
		return mDefaultIcon;
	}
}

bool VerbManager::IsVerb(const std::string& word)
{
	return mVerbs.find(word) != mVerbs.end();
}

bool VerbManager::IsInventoryItem(const std::string& word)
{
	return mInventoryItems.find(word) != mInventoryItems.end();
}

bool VerbManager::IsTopic(const std::string& word)
{
	return mTopics.find(word) != mTopics.end();
}
