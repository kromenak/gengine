//
// Clark Kromenaker
//
// UI screen that displays current inventory and inspects inventory items.
//
#pragma once
#include "Actor.h"

#include <set>
#include <string>
#include <vector>

#include "InventoryManager.h"
#include "LayerManager.h"

class UIButton;
class UIImage;

class InventoryScreen : public Actor
{
public:
	InventoryScreen();
	
	void Show(const std::string& actorName, const std::set<std::string>& inventory);
	void Hide();
    bool IsShowing() const;

    void RefreshLayout();
	
private:
	const int kActiveHighlightXOffset = -4;
    
    // This screen's layer.
    Layer mLayer;
    
	// Created item buttons - can reuse on repeat visits.
	std::vector<UIButton*> mItemButtons;
	
	// A highlight that appears over the active inventory item.
	UIImage* mActiveHighlightImage = nullptr;
	
	// Actor and inventory we are looking at.
	std::string mCurrentActorName;
    const std::set<std::string>* mCurrentInventory = nullptr;
	
	void OnItemClicked(UIButton* button, std::string itemName);
};
