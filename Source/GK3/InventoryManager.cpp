#include "InventoryManager.h"

#include "AssetManager.h"
#include "IniParser.h"
#include "InventoryScreen.h"
#include "InventoryInspectScreen.h"
#include "Loader.h"
#include "Profiler.h"
#include "Scene.h"
#include "StringUtil.h"
#include "TextAsset.h"
#include "Texture.h"

InventoryManager gInventoryManager;

void InventoryManager::Init()
{
    TIMER_SCOPED("InventoryManager::InventoryManager");

    Loader::Load([this]() {
        TIMER_SCOPED("InventoryManager::Load");

        // Get VERBS text file as a raw buffer.
        TextAsset* textFile = gAssetManager.LoadText("INVENTORYSPRITES.TXT", AssetScope::Manual);

        // Pass that along to INI parser, since it is plain text and in INI format.
        IniParser parser(textFile->GetText(), textFile->GetTextLength());

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

                // Save to map.
                mInventoryItems.emplace(std::make_pair(entry.key, entry.value));
            }
        }
        delete textFile;
    });
    
	// Create inventory screen UI.
	mInventoryScreen = new InventoryScreen();
	mInventoryScreen->SetIsDestroyOnLoad(false);
	
	// Create inventory inspect UI.
	mInventoryInspectScreen = new InventoryInspectScreen();
	mInventoryInspectScreen->SetIsDestroyOnLoad(false);
	
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
	
	// Give Grace starting inventory items.
    AddInventoryItem("GRACE", "GRACE_WALLET");
    AddInventoryItem("GRACE", "GRACES_PASSPORT");
    AddInventoryItem("GRACE", "R25_ROOM_KEY");
    AddInventoryItem("GRACE", "SKETCHPAD");
}

bool InventoryManager::IsValidInventoryItem(const std::string& itemName) const
{
	return mInventoryItems.find(itemName) != mInventoryItems.end();
}

void InventoryManager::AddInventoryItem(const std::string& itemName)
{
    AddInventoryItem(Scene::GetEgoName(), itemName);
}

void InventoryManager::AddInventoryItem(const std::string& actorName, const std::string& itemName)
{
    // Add to inventory.
	std::set<std::string>& items = mInventories[actorName];
	items.insert(StringUtil::ToUpperCopy(itemName));

    // Inventory item can be added while screen is showing (ex: combining items to create a new item).
    if(mInventoryScreen->IsActive())
    {
        mInventoryScreen->RefreshLayout();
    }
}

void InventoryManager::RemoveInventoryItem(const std::string& itemName)
{
    RemoveInventoryItem(Scene::GetEgoName(), itemName);
}

void InventoryManager::RemoveInventoryItem(const std::string& actorName, const std::string& itemName)
{
    // Remove from inventory.
	std::set<std::string>& items = mInventories[actorName];
	items.erase(StringUtil::ToUpperCopy(itemName));

    // If this was the active inventory item, clear that.
    if(StringUtil::EqualsIgnoreCase(GetActiveInventoryItem(actorName), itemName))
    {
        SetActiveInventoryItem(actorName, "");
    }

    // Inventory item can be removed while screen is showing (ex: eating the candy).
    if(mInventoryScreen->IsActive())
    {
        mInventoryScreen->RefreshLayout();
    }
}

bool InventoryManager::HasInventoryItem(const std::string& itemName) const
{
    return HasInventoryItem(Scene::GetEgoName(), itemName);
}

bool InventoryManager::HasInventoryItem(const std::string& actorName, const std::string& itemName) const
{
	auto it = mInventories.find(actorName);
	if(it == mInventories.end()) { return false; }
	return it->second.find(StringUtil::ToUpperCopy(itemName)) != it->second.end();
}

std::string InventoryManager::GetActiveInventoryItem() const
{
    return GetActiveInventoryItem(Scene::GetEgoName());
}

std::string InventoryManager::GetActiveInventoryItem(const std::string& actorName) const
{
	auto it = mActiveInventoryItems.find(actorName);
	if(it == mActiveInventoryItems.end())
	{
		return "";
	}
	return it->second;
}

void InventoryManager::SetActiveInventoryItem(const std::string& actorName, const std::string& itemName)
{
	mActiveInventoryItems[actorName] = itemName;

    // Active item can be changed via sheep calls while inventory is showing.
    if(mInventoryScreen->IsActive())
    {
        mInventoryScreen->RefreshLayout();
    }
}

void InventoryManager::ShowInventory()
{
    ShowInventory(Scene::GetEgoName());
}

void InventoryManager::ShowInventory(const std::string& actorName)
{
	mInventoryScreen->Show(actorName, mInventories[actorName]);
}

void InventoryManager::HideInventory()
{
	mInventoryScreen->Hide();
}

bool InventoryManager::IsInventoryShowing() const
{
	return mInventoryScreen->IsShowing();
}

void InventoryManager::InventoryInspect()
{
    InventoryInspect(GetActiveInventoryItem());
}

void InventoryManager::InventoryInspect(const std::string& itemName)
{
	mInventoryInspectScreen->Show(itemName);
}

void InventoryManager::InventoryUninspect()
{
	mInventoryInspectScreen->Hide();
}

bool InventoryManager::IsInventoryInspectShowing() const
{
    return mInventoryInspectScreen->IsShowing();
}

Texture* InventoryManager::GetInventoryItemIconTexture(const std::string& itemName)
{
    // Find the item. If doesn't exist, return null.
    auto it = mInventoryItems.find(itemName);
    if(it == mInventoryItems.end()) { return nullptr; }

    // Return cached texture if already loaded.
    if(it->second.iconTexture != nullptr) { return it->second.iconTexture; }

    // Load the texture fresh.
    it->second.iconTexture = gAssetManager.LoadTexture(it->second.textureNamePrefix + "3.BMP");
    if(it->second.iconTexture == nullptr)
    {
        it->second.iconTexture = gAssetManager.LoadTexture(it->second.textureNamePrefix + "_3.BMP");
    }
    return it->second.iconTexture;
}

Texture* InventoryManager::GetInventoryItemListTexture(const std::string& itemName)
{
    // Find the item. If doesn't exist, return null.
    auto it = mInventoryItems.find(itemName);
    if(it == mInventoryItems.end()) { return nullptr; }

    // Return cached texture if already loaded.
    if(it->second.listTexture != nullptr) { return it->second.listTexture; }

    // Otherwise, load the texture!
    // List texture has a "9" suffix. Also, optionally, an alpha texture (since these show against a see-through background).
    it->second.listTexture = gAssetManager.LoadTexture(it->second.textureNamePrefix + "9.BMP");
    if(it->second.listTexture == nullptr)
    {
        it->second.listTexture = gAssetManager.LoadTexture(it->second.textureNamePrefix + "_9.BMP");
    }

    // If we have a list texture, attempt to find and apply alpha channel.
    if(it->second.listTexture != nullptr)
    {
        Texture* listTextureAlpha = gAssetManager.LoadTexture(it->second.textureNamePrefix + "9_OP.BMP");
        if(listTextureAlpha == nullptr)
        {
            listTextureAlpha = gAssetManager.LoadTexture(it->second.textureNamePrefix + "_9_OP.BMP");
        }
        if(listTextureAlpha != nullptr)
        {
            it->second.listTexture->ApplyAlphaChannel(*listTextureAlpha);
        }
    }
    return it->second.listTexture;
}

Texture* InventoryManager::GetInventoryItemCloseupTexture(const std::string& itemName)
{
    // Find the item. If doesn't exist, return null.
    auto it = mInventoryItems.find(itemName);
    if(it == mInventoryItems.end()) { return nullptr; }

    // Return cached texture if already loaded.
    if(it->second.closeupTexture != nullptr) { return it->second.closeupTexture; }

    // Closeup texture has suffix "6" or "6_ALPHA".
    // One asset (MOSELYPRINT_6_ALPHA.BMP) did not follow the naming convention - yuck.
    it->second.closeupTexture = gAssetManager.LoadTexture(it->second.textureNamePrefix + "6.BMP");
    if(it->second.closeupTexture == nullptr)
    {
        it->second.closeupTexture = gAssetManager.LoadTexture(it->second.textureNamePrefix + "6_ALPHA.BMP");
        if(it->second.closeupTexture == nullptr)
        {
            it->second.closeupTexture = gAssetManager.LoadTexture(it->second.textureNamePrefix + "_6_ALPHA.BMP");
        }
    }
    return it->second.closeupTexture;
}
