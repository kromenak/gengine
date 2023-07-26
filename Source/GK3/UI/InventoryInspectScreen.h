//
// Clark Kromenaker
//
// UI screen that shows a closeup image of an inventory item.
//
#pragma once
#include "Actor.h"

#include "LayerManager.h"

class UIButton;

class InventoryInspectScreen : public Actor
{
public:
	InventoryInspectScreen();
	
	void Show(const std::string& itemName);
	void Hide();
    bool IsShowing() const;
	
private:
    // This screen's layer.
    Layer mLayer;
    
	// Image to display the closeup of the item.
	// Needs to be a button b/c we can click to show action bar.
	UIButton* mCloseupImage = nullptr;
	
	// The name of the item currently being inspected.
	std::string mInspectItemName;
	
	void OnClicked();
};
