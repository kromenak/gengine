//
// InventoryManager.cpp
//
// Clark Kromenaker
//
#include "InventoryManager.h"

#include "IniParser.h"
#include "Services.h"
#include "StringUtil.h"

TYPE_DEF_BASE(InventoryManager);

InventoryManager::InventoryManager()
{
	// Get VERBS text file as a raw buffer.
	unsigned int bufferSize = 0;
	char* buffer = Services::GetAssets()->LoadRaw("INVENTORYSPRITES.TXT", bufferSize);
	
	// Pass that along to INI parser, since it is plain text and in INI format.
	IniParser parser(buffer, bufferSize);
	
	// There's only one unnamed section in this file.
	while(parser.ReadLine())
	{
		while(parser.ReadKeyValuePair())
	    {
			const IniKeyValue& entry = parser.GetKeyValue();
			
			// INVENTORYSPRITES.TXT is the ONLY INI-style text file I've seen in GK3 that
			// uses "; //" for comments instead of just "//".
			// Rather than muck up the parser code, I'll just catch and ignore those lines here.
			if(entry.key[0] == ';') { continue; }
			
			//TODO: Use entry.value to populate InventoryItemTextures.
			mInventoryItems[StringUtil::ToLowerCopy(entry.key)] = InventoryItemTextures();
			std::cout << "Added " << entry.key << std::endl;
	    }
	}
}

bool InventoryManager::IsValidInventoryItem(const std::string& itemName) const
{
	std::string itemNameLower = StringUtil::ToLowerCopy(itemName);
	return mInventoryItems.find(itemNameLower) != mInventoryItems.end();
}

void InventoryManager::AddInventoryItem(const std::string& actorName, const std::string& itemName)
{
	std::string actorNameLower = StringUtil::ToLowerCopy(actorName);
	std::string itemNameLower = StringUtil::ToLowerCopy(itemName);
	std::unordered_set<std::string>& items = mInventories[actorNameLower];
	items.insert(itemNameLower);
}

void InventoryManager::RemoveInventoryItem(const std::string& actorName, const std::string& itemName)
{
	std::string actorNameLower = StringUtil::ToLowerCopy(actorName);
	std::string itemNameLower = StringUtil::ToLowerCopy(itemName);
	std::unordered_set<std::string>& items = mInventories[actorNameLower];
	items.erase(itemNameLower);
}

bool InventoryManager::HasInventoryItem(const std::string& actorName, const std::string& itemName) const
{
	std::string actorNameLower = StringUtil::ToLowerCopy(actorName);
	std::string itemNameLower = StringUtil::ToLowerCopy(itemName);
	auto it = mInventories.find(actorNameLower);
	if(it == mInventories.end()) { return false; }
	return it->second.find(itemNameLower) != it->second.end();
}

std::string InventoryManager::GetActiveInventoryItem(const std::string& actorName) const
{
	std::string actorNameLower = StringUtil::ToLowerCopy(actorName);
	auto it = mActiveInventoryItems.find(actorNameLower);
	if(it == mActiveInventoryItems.end())
	{
		return "";
	}
	return it->second;
}

void InventoryManager::SetActiveInventoryItem(const std::string& actorName, const std::string& itemName)
{
	std::string actorNameLower = StringUtil::ToLowerCopy(actorName);
	std::string itemNameLower = StringUtil::ToLowerCopy(itemName);
	mActiveInventoryItems[actorNameLower] = itemNameLower;
}

