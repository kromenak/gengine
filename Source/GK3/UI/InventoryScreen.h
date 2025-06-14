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

#include "LayerManager.h"

class UIButton;
class UIImage;
class UIScrollbar;

class InventoryScreen : public Actor
{
public:
    InventoryScreen();

    void Show(const std::string& actorName, const std::set<std::string>& inventory);
    void Hide();
    bool IsShowing() const;

    void RefreshLayout();

protected:
    void OnUpdate(float deltaTime) override;

private:
    const int kActiveHighlightXOffset = -4;

    // This screen's layer.
    Layer mLayer;

    // The exit button on this screen.
    UIButton* mExitButton = nullptr;

    // Scrollbar on the right side of the screen; allows scrolling when too many items for one screen.
    UIScrollbar* mScrollbar = nullptr;

    // Created item buttons - can reuse on repeat visits.
    std::vector<UIButton*> mItemButtons;

    // A highlight that appears over the active inventory item.
    UIImage* mActiveHighlightImage = nullptr;

    // Actor and inventory we are looking at.
    std::string mCurrentActorName;
    const std::set<std::string>* mCurrentInventory = nullptr;

    // The offset for which row of inventory items is at the top of the inventory screen.
    // When there are too many items to fit on one screen, you can "scroll down" to see off-screen rows.
    int mScrollRowOffset = 0;

    // The maximum scroll row offset that's allowed. At this offset, you're seeing the last page of inventory items.
    int mMaxScrollRowOffset = 0;

    void OnItemClicked(UIButton* button, const std::string& itemName);

    void OnScrollbarUpArrowPressed();
    void OnScrollbarDownArrowPressed();
    void OnScrollbarValueChanged(float value);
};
