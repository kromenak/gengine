//
// Clark Kromenaker
//
// Maps inventory items and inventory item states to actors in the game.
//
// Inventory items are only represented internally as strings. Likewise, actors
// are represented by their names.
//
// As a result, tracking who has what items or the statuses of items is very simple.
// However, using invalid or unsupported inventory items is probably a bad idea because
// the necessary art assets won't be available.
//
#pragma once
#include <set>
#include <string>
#include <unordered_map>

#include "StringUtil.h"

class InventoryScreen;
class InventoryInspectScreen;
class Texture;

class InventoryManager
{
public:
    void Init();
	
	bool IsValidInventoryItem(const std::string& itemName) const;

    void AddInventoryItem(const std::string& itemName);
	void AddInventoryItem(const std::string& actorName, const std::string& itemName);

    void RemoveInventoryItem(const std::string& itemName);
	void RemoveInventoryItem(const std::string& actorName, const std::string& itemName);

    bool HasInventoryItem(const std::string& itemName) const;
	bool HasInventoryItem(const std::string& actorName, const std::string& itemName) const;

    std::string GetActiveInventoryItem() const;
	std::string GetActiveInventoryItem(const std::string& actorName) const;
	void SetActiveInventoryItem(const std::string& actorName, const std::string& itemName);
	
    void ShowInventory();
	void ShowInventory(const std::string& actorName);
	void HideInventory();
	bool IsInventoryShowing() const;

    void InventoryInspect();
	void InventoryInspect(const std::string& itemName);
	void InventoryUninspect();
    bool IsInventoryInspectShowing() const;

    Texture* GetInventoryItemIconTexture(const std::string& itemName);
	Texture* GetInventoryItemListTexture(const std::string& itemName);
	Texture* GetInventoryItemCloseupTexture(const std::string& itemName);
	
private:
	// Maps an inventory item's name to its graphical representations.
	// Allows us to look up inventory textures AND determine whether an inventory item is "valid".
    struct InventoryItemTextures
    {
        // String used as a prefix to load these textures.
        std::string textureNamePrefix;

        // Icon texture: appears on the option bar as the active inventory item.
        // Smallest image, not a lot of detail.
        Texture* iconTexture = nullptr;

        // List texture: appears on the inventory screen, in the list.
        // Medium-size image, with alpha layer usually.
        Texture* listTexture = nullptr;

        // Closeup texture: appears in the inventory item closeup screen.
        // Largest image, most detail.
        Texture* closeupTexture = nullptr;

        InventoryItemTextures() = default;
        InventoryItemTextures(const std::string& prefix) : textureNamePrefix(prefix) { }
    };
	std::string_map_ci<InventoryItemTextures> mInventoryItems;
	
	// Inventories for actors.
	// Key is actor name, value is a set. If a value is present in the set, the actor "has" that inventory item.
	// Set is used (rather than unordered_set) b/c we want items to display in UI in alphabetical order.
	std::string_map_ci<std::set<std::string>> mInventories;
	
	// Each actor can have one active inventory item.
	// It's also possible for an actor to have NO active item!
	std::string_map_ci<std::string> mActiveInventoryItems;
    
	// UI for inventory.
	InventoryScreen* mInventoryScreen = nullptr;
	InventoryInspectScreen* mInventoryInspectScreen = nullptr;
};

extern InventoryManager gInventoryManager;