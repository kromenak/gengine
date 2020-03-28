//
// InventoryInspectScreen.h
//
// Clark Kromenaker
//
// UI screen that shows a closeup image of an inventory item.
//
#pragma once
#include "Actor.h"

class UICanvas;
class UIButton;

class InventoryInspectScreen : public Actor
{
public:
	InventoryInspectScreen();
	
	void Show(const std::string& itemName);
	void Hide();
	bool IsShowing() const { return IsActive(); }
	
private:
	// Canvas to render UI.
	UICanvas* mCanvas = nullptr;
	
	// Image to display the closeup of the item.
	// Needs to be a button b/c we can click to show action bar.
	UIButton* mCloseupImage = nullptr;
	
	// The name of the item currently being inspected.
	std::string mInspectItemName;
	
	void OnClicked();
};
