//
// InventoryManager.h
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
#pragma once
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "Type.h"

struct InventoryItemTextures
{
	/*
	 TODO: INVENTORY_SPRITES.TXT defines a map from item name to texture prefix.
	 TODO: There are then 4 textures with that prefix in the asset catalog:
	 VITPRNTWILKESNAME3.BMP
	 VITPRNTWILKESNAME6_ALPHA.BMP
	 VITPRNTWILKESNAME9.BMP
	 VITPRNTWILKESNAME9_OP.BMP
	 */
};

class InventoryManager
{
	TYPE_DECL_BASE();
public:
	InventoryManager();
	
	bool IsValidInventoryItem(const std::string& itemName) const;
	
	void AddInventoryItem(const std::string& actorName, const std::string& itemName);
	void RemoveInventoryItem(const std::string& actorName, const std::string& itemName);
	bool HasInventoryItem(const std::string& actorName, const std::string& itemName) const;
	
	std::string GetActiveInventoryItem(const std::string& actorName) const;
	void SetActiveInventoryItem(const std::string& actorName, const std::string& itemName);
	
	void ShowInventory() const;
	void HideInventory() const;
	
	void InventoryInspect(const std::string& itemName) const;
	void InventoryUninspect() const;
	
private:
	// A map from name to textures used for that inventory item.
	// Allows us to look up inventory textures AND determine whether an inventory item is "valid".
	std::unordered_map<std::string, InventoryItemTextures> mInventoryItems;
	
	// Inventories for actors.
	// Key is actor name, value is a set. If a value is present in the set, the actor "has" that inventory item.
	std::unordered_map<std::string, std::unordered_set<std::string>> mInventories;
	
	// Each actor can have one active inventory item.
	// It's also possible for an actor to have NO active item!
	std::unordered_map<std::string, std::string> mActiveInventoryItems;
};
