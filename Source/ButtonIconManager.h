//
// ButtonIconManager.h
//
// Clark Kromenaker
//
// GK3 stores a mapping from Noun/Verb/Topic to button icons
// in a text asset called VERBS.TXT. My guess is that it was
// initially a mapping of verbs to button icons, but eventually
// grew to also include Inventory Nouns and Conversation Topic icons.
//
// This manager just loads these assets into memory (using VERBS.TXT)
// and makes them easily accessible for other systems to access.
//
#pragma once
#include <string>
#include <unordered_map>

#include "Type.h"

class Texture;

struct ButtonIcon
{
	Texture* upTexture = nullptr;
	Texture* downTexture = nullptr;
	Texture* hoverTexture = nullptr;
	Texture* disableTexture = nullptr;
	
	float GetWidth() const;
};

class ButtonIconManager
{
	TYPE_DECL_BASE();
public:
	ButtonIconManager();
	
	ButtonIcon& GetButtonIconForNoun(const std::string& noun);
	ButtonIcon& GetButtonIconForVerb(const std::string& verb);
	ButtonIcon& GetButtonIconForTopic(const std::string& topic);
	
	bool IsInventoryItem(const std::string& word);
	
private:
	ButtonIcon mDefaultIcon;
	
	// Mappings from noun/verb/topic to icons.
	std::unordered_map<std::string, ButtonIcon> mNounsToIcons;
	std::unordered_map<std::string, ButtonIcon> mVerbsToIcons;
	std::unordered_map<std::string, ButtonIcon> mTopicsToIcons;
};
