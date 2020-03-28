//
// InventoryManager.cpp
//
// Clark Kromenaker
//
#include "InventoryManager.h"

#include "IniParser.h"
#include "InventoryScreen.h"
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
			
			// Populate a textures entry for this inventory item.
			InventoryItemTextures textures;
			
			// Closeup texture has suffix "6_ALPHA" after entry value.
			// Why "6_ALPHA"? Who knows! Sometimes it's even "_6_ALPHA"!
			textures.closeupTexture = Services::GetAssets()->LoadTexture(entry.value + "6_ALPHA.BMP");
			if(textures.closeupTexture == nullptr)
			{
				textures.closeupTexture = Services::GetAssets()->LoadTexture(entry.value + "_6_ALPHA.BMP");
			}
			
			// List texture has suffix "9" - again, why? Who knows!
			// List texture also has an alpha texture.
			// In both cases, the leading underscore is inconsistently used, so we must check for both.
			textures.listTexture = Services::GetAssets()->LoadTexture(entry.value + "9.BMP");
			if(textures.listTexture == nullptr)
			{
				textures.listTexture = Services::GetAssets()->LoadTexture(entry.value + "_9.BMP");
			}
			Texture* listTextureAlpha = Services::GetAssets()->LoadTexture(entry.value + "9_OP.BMP");
			if(listTextureAlpha == nullptr)
			{
				listTextureAlpha = Services::GetAssets()->LoadTexture(entry.value + "_9_OP.BMP");
			}
			
			// Apply alpha channel to list texture, if we have it.
			if(textures.listTexture != nullptr && listTextureAlpha != nullptr)
			{
				textures.listTexture->ApplyAlphaChannel(*listTextureAlpha);
				textures.listTexture->UploadToGPU();
			}
			
			// Save to map.
			mInventoryItems[StringUtil::ToLowerCopy(entry.key)] = textures;
	    }
	}
	delete[] buffer;
	
	// Create inventory screen UI.
	mInventoryScreen = new InventoryScreen();
	mInventoryScreen->SetIsDestroyOnLoad(false);
	
	// Give Gabe starting inventory items.
	//TODO: Obviously, this should be data-driven, but not sure where this is defined?
	AddInventoryItem("GABRIEL", "DAGGER");
	AddInventoryItem("GABRIEL", "GABES_PASSPORT");
	AddInventoryItem("GABRIEL", "NOTEPAD");
	AddInventoryItem("GABRIEL", "PRINCE_JAMES_CARD");
	AddInventoryItem("GABRIEL", "R25_ROOM_KEY");
	AddInventoryItem("GABRIEL", "TALISMAN");
	AddInventoryItem("GABRIEL", "TAPE_RECORDER");
	AddInventoryItem("GABRIEL", "WALLET");
}

Texture* InventoryManager::GetInventoryItemListTexture(const std::string &itemName)
{
	return mInventoryItems[StringUtil::ToLowerCopy(itemName)].listTexture;
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
	std::set<std::string>& items = mInventories[actorNameLower];
	items.insert(itemNameLower);
}

void InventoryManager::RemoveInventoryItem(const std::string& actorName, const std::string& itemName)
{
	std::string actorNameLower = StringUtil::ToLowerCopy(actorName);
	std::string itemNameLower = StringUtil::ToLowerCopy(itemName);
	std::set<std::string>& items = mInventories[actorNameLower];
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
	std::string activeInvKey = StringUtil::ToLowerCopy(actorName);
	auto it = mActiveInventoryItems.find(activeInvKey);
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

void InventoryManager::ShowInventory(const std::string& actorName)
{
	std::string invKey = StringUtil::ToLowerCopy(actorName);
	mInventoryScreen->Show(actorName, mInventories[invKey]);
}

void InventoryManager::HideInventory() const
{
	mInventoryScreen->Hide();
}

bool InventoryManager::IsInventoryShowing() const
{
	return mInventoryScreen->IsShowing();
}

void InventoryManager::InventoryInspect(const std::string& itemName) const
{
	std::cout << "Inventory Inspect " << itemName << std::endl;
}

void InventoryManager::InventoryUninspect() const
{
	std::cout << "Inventory Uninspect" << std::endl;
}
