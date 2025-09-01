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

    void TurnLSRPageLeft();
    void TurnLSRPageRight();

protected:
    void OnUpdate(float deltaTime) override;

private:
    // This screen's layer.
    Layer mLayer;

    // The exit button on this screen.
    UIButton* mExitButton = nullptr;

    // Image to display the closeup of the item.
    // Needs to be a button b/c we can click to show action bar.
    UIButton* mCloseupImage = nullptr;

    // The name of the item currently being inspected.
    std::string mInspectItemName;

    // If true, we are inspecting an item that is in our inventory.
    bool mInspectingItemFromInventory = true;

    // For LSR in particular, we need additional buttons.
    UIButton* mLSRButtons[3] = { nullptr };

    // For LSR, keep track of what page we're on.
    int mLSRPage = 1;

    void OnClicked(const std::string& noun);

    bool IsDemoIntro() const;
    bool IsLSR() const;
    void InitLSR();
};
