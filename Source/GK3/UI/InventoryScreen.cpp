#include "InventoryScreen.h"

#include "ActionBar.h"
#include "ActionManager.h"
#include "AssetManager.h"
#include "GK3UI.h"
#include "InputManager.h"
#include "InventoryManager.h"
#include "RectTransform.h"
#include "StringUtil.h"
#include "Texture.h"
#include "UIButton.h"
#include "UIImage.h"
#include "UIScrollbar.h"
#include "UIUtil.h"
#include "Window.h"

InventoryScreen::InventoryScreen() : Actor("InventoryScreen", TransformType::RectTransform),
    mLayer("InventoryLayer")
{
    // Inventory overrides SFX/VO, but continues ambient audio from scene.
    mLayer.OverrideAudioState(true, true, false);

    // Add canvas for rendering UI elements. Background is tinted.
    UI::AddCanvas(this, 5, Color32(0, 0, 0, 128));

    // Add exit button to bottom-left corner of screen.
    UIButton* exitButton = UI::CreateWidgetActor<UIButton>("ExitButton", this);
    exitButton->SetUpTexture(gAssetManager.LoadTexture("EXITN.BMP"));
    exitButton->SetDownTexture(gAssetManager.LoadTexture("EXITD.BMP"));
    exitButton->SetHoverTexture(gAssetManager.LoadTexture("EXITHOV.BMP"));
    exitButton->SetDisabledTexture(gAssetManager.LoadTexture("EXITDIS.BMP"));
    exitButton->SetPressCallback([this](UIButton* button){ Hide(); });
    exitButton->SetTooltipText("inventoryexit");
    exitButton->GetRectTransform()->SetAnchor(AnchorPreset::BottomLeft);
    exitButton->GetRectTransform()->SetAnchoredPosition(10.0f, 10.0f);
    mExitButton = exitButton;

    // Add scrollbar on the right side of the screen, for when there are too many
    // inventory items to fit on one screen.
    UIScrollbarParams scrollbarParams;
    scrollbarParams.decreaseValueButtonUp = gAssetManager.LoadTexture("INV_SCROLLUP_STD.BMP");
    scrollbarParams.decreaseValueButtonDown = gAssetManager.LoadTexture("INV_SCROLLUP_DWN.BMP");
    scrollbarParams.increaseValueButtonUp = gAssetManager.LoadTexture("INV_SCROLLDN_STD.BMP");
    scrollbarParams.increaseValueButtonDown = gAssetManager.LoadTexture("INV_SCROLLDN_DWN.BMP");
    scrollbarParams.scrollbarBacking = gAssetManager.LoadTexture("INV_SCROLLBACK.BMP");
    scrollbarParams.handleParams.leftColor = scrollbarParams.handleParams.topColor = scrollbarParams.handleParams.topLeftColor = Color32(181, 125, 0);
    scrollbarParams.handleParams.rightColor = scrollbarParams.handleParams.bottomColor = scrollbarParams.handleParams.topRightColor =
        scrollbarParams.handleParams.bottomRightColor = scrollbarParams.handleParams.bottomLeftColor = Color32(90, 28, 33);
    scrollbarParams.handleParams.centerColor = Color32(123, 77, 8);
    scrollbarParams.handleParams.borderWidth = 2;

    mScrollbar = UI::CreateWidgetActor<UIScrollbar>("Scrollbar", this, scrollbarParams);
    mScrollbar->SetDecreaseValueCallback([this](){
        OnScrollbarUpArrowPressed();
    });
    mScrollbar->SetIncreaseValueCallback([this](){
        OnScrollbarDownArrowPressed();
    });
    mScrollbar->SetValueChangeCallback([this](float value){
        OnScrollbarValueChanged(value);
    });

    // Create active inventory item highlight, but hide by default.
    mActiveHighlightImage = UI::CreateWidgetActor<UIImage>("ActiveHighlight", this);
    mActiveHighlightImage->SetTexture(gAssetManager.LoadTexture("INV_HIGHLIGHT.BMP"), true);
    mActiveHighlightImage->SetEnabled(false);
    mActiveHighlightImage->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);

    // Hide inventory UI by default.
    SetActive(false);
}

void InventoryScreen::Show(const std::string& actorName, const std::set<std::string>& inventory)
{
    // Already showing, so don't do it again!
    if(IsActive()) { return; }

    // Push layer onto stack.
    gLayerManager.PushLayer(&mLayer);

    // Show the screen.
    // Be sure to do this before refreshing the layout, so that the canvas sizing/scaling is accurate.
    SetActive(true);

    // Save current actor name and inventory.
    mCurrentActorName = actorName;
    mCurrentInventory = &inventory;

    // The scroll offset always resets when showing this screen.
    mScrollRowOffset = 0;

    // Layout the buttons on screen.
    RefreshLayout();
}

void InventoryScreen::Hide()
{
    if(!IsActive()) { return; }
    SetActive(false);

    // Pop off stack.
    gLayerManager.PopLayer(&mLayer);

    // Clear inventory data.
    mCurrentActorName.clear();
    mCurrentInventory = nullptr;
}

bool InventoryScreen::IsShowing() const
{
    return gLayerManager.IsTopLayer(&mLayer);
}

void InventoryScreen::RefreshLayout()
{
    // Hide active inventory highlight by default.
    mActiveHighlightImage->SetEnabled(false);

    // Hide all pre-existing buttons.
    for(auto& button : mItemButtons)
    {
        button->SetEnabled(false);
    }

    // Need an inventory to populate.
    if(mCurrentInventory == nullptr)
    {
        return;
    }

    // Get active inventory item for actor.
    std::string activeInventoryItem = gInventoryManager.GetActiveInventoryItem(mCurrentActorName);

    // These constants control the positioning of inventory items in the grid.
    const float kStartX = 60.0f;
    const float kStartY = -90.0f;
    const float kSpacingX = 10.0f;
    const float kSpacingY = 10.0f;

    // These constants control how close to the right/bottom screen edges inventory items can get.
    RectTransform* rt = static_cast<RectTransform*>(GetTransform());
    const float kMaxX = rt->GetSizeDelta().x * 0.9f;
    const float kMaxY = -(rt->GetSizeDelta().y * 0.75f);

    // As we calculate the position of each inventory item, keep track of the current row.
    // For scrolling purposes, also track the first and last row that are visible on-screen.
    int rowIndex = 0;
    int firstOnScreenRowIndex = -1;
    int lastOnScreenRowIndex = 0;

    // Populate the inventory screen.
    float x = kStartX;
    float y = kStartY;
    size_t buttonIndex = 0;
    for(auto& item : *mCurrentInventory)
    {
        // Get item texture.
        // The system returns a placeholder texture if inv item can't be found, so we always have a valid texture here.
        Texture* itemTexture = gInventoryManager.GetInventoryItemListTexture(item);

        // If this next item will go offscreen, we should move down to next row.
        if(x + itemTexture->GetWidth() > kMaxX)
        {
            x = kStartX;
            y -= (itemTexture->GetHeight() + kSpacingY);
            ++rowIndex;
        }

        // The *actual* y-position of the item depends on the current scroll offset.
        float actualY = y + (mScrollRowOffset * (itemTexture->GetHeight() + kSpacingY));

        // If true, this inventory item is on-screen (not scrolled off-screen), so we need a button for it.
        bool createButton = actualY <= kStartY && actualY >= kMaxY;
        if(createButton)
        {
            // As we create buttons, keep track of the first & last rows that have actual on-screen buttons.
            if(firstOnScreenRowIndex == -1)
            {
                firstOnScreenRowIndex = rowIndex;
            }
            lastOnScreenRowIndex = rowIndex;

            // Either reuse an already created button or create a new one.
            UIButton* button = nullptr;
            if(buttonIndex < mItemButtons.size())
            {
                button = mItemButtons[buttonIndex];
                button->SetEnabled(true);
            }
            else
            {
                button = UI::CreateWidgetActor<UIButton>("InvItem", this);
                button->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
                mItemButtons.push_back(button);
            }
            ++buttonIndex;

            // Set position/size for button.
            button->GetRectTransform()->SetAnchoredPosition(x, actualY);
            button->GetRectTransform()->SetSizeDelta(itemTexture->GetWidth(), itemTexture->GetHeight());

            // Set texture for button.
            button->SetUpTexture(itemTexture);

            // Set button callback.
            button->SetPressCallback([this, item](UIButton* button){ OnItemClicked(button, item); });

            // See if this is the active inventory item.
            // If so, position the highlight over it.
            if(StringUtil::EqualsIgnoreCase(item, activeInventoryItem))
            {
                RectTransform* activeHighlightRT =
                    mActiveHighlightImage->GetRectTransform();
                activeHighlightRT->SetAnchoredPosition(x + kActiveHighlightXOffset, actualY + 2); // nudge up slightly for better positioning
                mActiveHighlightImage->SetEnabled(true);
            }
        }

        // Next button located to the right, with spacing.
        x += itemTexture->GetWidth() + kSpacingX;
    }

    // After iterating all inventory items, we can calculate how many rows of items ARE NOT on screen.
    // If there are any, then we must show the scrollbar!
    int rowsNotOnScreen = firstOnScreenRowIndex + (rowIndex - lastOnScreenRowIndex);
    mScrollbar->GetOwner()->SetActive(rowsNotOnScreen > 0);

    // Remember how many rows aren't on screen, as that's the maximum row offset we can use for scrolling.
    mMaxScrollRowOffset = rowsNotOnScreen;

    // Make sure scrollbar appearance/value are correct.
    if(rowsNotOnScreen > 0)
    {
        // The size of the scrollbar handle changes based on how many rows aren't on-screen.
        // An easy way to do this is to just make the % size of handle decrease as more rows are not on screen.
        float normalizedScrollbarHandleSize = 1.0f / rowsNotOnScreen;
        mScrollbar->SetHandleNormalizedSize(normalizedScrollbarHandleSize);

        // Make sure scrollbar is in the correct spot.
        mScrollbar->SetValueSilently(static_cast<float>(mScrollRowOffset) / mMaxScrollRowOffset);
    }
}

void InventoryScreen::OnUpdate(float deltaTime)
{
    // Escape key is a shortcut to exit. But be sure this screen's on the top of all others and no action is playing.
    if(gGK3UI.CanExitScreen(mLayer) && gInputManager.IsKeyLeadingEdge(SDL_SCANCODE_ESCAPE))
    {
        mExitButton->AnimatePress();
    }

    // If scrollbar is active, and not being dragged, snap the scrollbar to the value corresponding to the current row offset.
    if(mScrollbar->IsActiveAndEnabled() && !mScrollbar->IsHandleBeingDragged())
    {
        mScrollbar->SetValueSilently(static_cast<float>(mScrollRowOffset) / mMaxScrollRowOffset);
    }
}

void InventoryScreen::OnItemClicked(UIButton* button, const std::string& itemName)
{
    // Show the action bar for this noun.
    gActionManager.ShowActionBar(itemName, true, nullptr);

    // We want to add a "pickup" verb, which means to make the item the active inventory item.
    // This should go after the "LOOK" verb, but if there's no "LOOK" verb, it'll go at the beginning.
    ActionBar* actionBar = gActionManager.GetActionBar();
    actionBar->AddVerbAtIndex("PICKUP", actionBar->GetVerbIndex("LOOK") + 1, [this, itemName](){
        gInventoryManager.SetActiveInventoryItem(this->mCurrentActorName, itemName);
    });

    // We want to also add an "inspect" verb, which means to show the close-up of the item.
    // This always goes at the front of the existing action bar.
    actionBar->AddVerbToFront("INSPECT", [itemName](){
        gInventoryManager.InventoryInspect(itemName);
    });
}

void InventoryScreen::OnScrollbarUpArrowPressed()
{
    // Decrement row offset change scrollbar value, and refresh the layout.
    if(mScrollRowOffset > 0)
    {
        --mScrollRowOffset;
        mScrollbar->SetValueSilently(static_cast<float>(mScrollRowOffset) / mMaxScrollRowOffset);
        RefreshLayout();
    }
}

void InventoryScreen::OnScrollbarDownArrowPressed()
{
    // Increment row offset change scrollbar value, and refresh the layout.
    if(mScrollRowOffset < mMaxScrollRowOffset)
    {
        ++mScrollRowOffset;
        mScrollbar->SetValueSilently(static_cast<float>(mScrollRowOffset) / mMaxScrollRowOffset);
        RefreshLayout();
    }
}

void InventoryScreen::OnScrollbarValueChanged(float value)
{
    // Unlike most scrollbars in the universe, this one is not "continuous" - it only scrolls the contents when you hit specific thresholds.
    // So first thing, figure out which threshold this value corresponds with.
    int oldRowOffset = mScrollRowOffset;
    for(int i = 0; i <= mMaxScrollRowOffset; ++i)
    {
        float normalizedValue = static_cast<float>(i) / mMaxScrollRowOffset;
        if(value >= normalizedValue)
        {
            // The scrollbar value is more than the value for this threshold, so we'll use this offset.
            mScrollRowOffset = i;
        }
        else
        {
            // The scrollbar is less than this threshold, so we don't need to iterate anymore.
            break;
        }
    }

    // If the offset changed due to changing the scrollbar value, refresh the layout.
    if(mScrollRowOffset != oldRowOffset)
    {
        RefreshLayout();
    }
}
