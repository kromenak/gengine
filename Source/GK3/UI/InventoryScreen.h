//
// InventoryScreen.h
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
class UICanvas;
class UIImage;

class InventoryScreen : public Actor
{
public:
	InventoryScreen();
	
	void Show(const std::string& actorName, const std::set<std::string>& inventory);
	void Hide();
    bool IsShowing() const;
	
private:
	const int kActiveHighlightXOffset = -4;
    
    // This screen's layer.
    Layer mLayer;
	
	// Canvas to render inventory UI.
	UICanvas* mCanvas = nullptr;
	
	// Created item buttons - can reuse on repeat visits.
	std::vector<UIButton*> mItemButtons;
	
	// A highlight that appears over the active inventory item.
	UIImage* mActiveHighlightImage = nullptr;
	
	// Name of the actor whose inventory we are currently viewing.
	std::string mCurrentActorName;
	
	void OnItemClicked(UIButton* button, std::string itemName);
};
