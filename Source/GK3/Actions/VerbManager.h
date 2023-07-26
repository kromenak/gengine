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

#include "StringUtil.h"
#include "Type.h"

class Texture;
class Cursor;

struct VerbIcon
{
    // Button textures for the verb.
	Texture* upTexture = nullptr;
	Texture* downTexture = nullptr;
	Texture* hoverTexture = nullptr;
	Texture* disableTexture = nullptr;

    // A cursor representing this verb.
    Cursor* cursor = nullptr;
	
    float GetWidth() const;
};

class VerbManager
{
public:
    void Init();
	
	VerbIcon& GetInventoryIcon(const std::string& noun);
	VerbIcon& GetVerbIcon(const std::string& verb);
	VerbIcon& GetTopicIcon(const std::string& topic);
	
	bool IsVerb(const std::string& word);
	bool IsInventoryItem(const std::string& word);
	bool IsTopic(const std::string& word);
	
private:
	VerbIcon mDefaultIcon;
	
	// Mappings from noun/verb/topic to icons.
	std::string_map_ci<VerbIcon> mInventoryItems;
	std::string_map_ci<VerbIcon> mVerbs;
	std::string_map_ci<VerbIcon> mTopics;
};

extern VerbManager gVerbManager;