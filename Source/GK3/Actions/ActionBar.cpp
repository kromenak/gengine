#include "ActionBar.h"

#include <algorithm>

#include "InventoryManager.h"
#include "Scene.h"
#include "StringUtil.h"
#include "UIButton.h"
#include "UICanvas.h"
#include "UILabel.h"
#include "VerbManager.h"
#include "Window.h"

ActionBar::ActionBar() : Actor(TransformType::RectTransform)
{
	// Create canvas, to contain the UI components.
	UICanvas* canvas = AddComponent<UICanvas>(5);
	
    // Canvas rect fills the entire screen.
    RectTransform* rectTransform = canvas->GetRectTransform();
	rectTransform->SetSizeDelta(0.0f, 0.0f);
	rectTransform->SetAnchorMin(Vector2::Zero);
	rectTransform->SetAnchorMax(Vector2::One);
    
    // The background of the action bar consists of a fullscreen clickable button area.
    // This stops interaction with the scene while action bar is visible.
    // It also allows clicking outside the bar to cancel it.
    mSceneBlockerButton = AddComponent<UIButton>();
    mSceneBlockerButton->SetPressCallback([this](UIButton* button) {
        if(mAllowDismiss)
        {
            OnCancelButtonPressed();
        }
    });
	
	// Create button holder - it holds the buttons and we move it around the screen.
    // Since we will set the action bar's position based on mouse position, set the anchor to the lower-left corner.
	Actor* buttonHolderActor = new Actor(TransformType::RectTransform);
	mButtonHolder = buttonHolderActor->GetComponent<RectTransform>();
	mButtonHolder->SetParent(rectTransform);
	mButtonHolder->SetAnchorMin(Vector2::Zero);
	mButtonHolder->SetAnchorMax(Vector2::Zero);
	mButtonHolder->SetPivot(0.5f, 0.5f);
	
	// To have button holder appear in correct spot, we need the holder to be the right height.
	// So, just use one of the buttons to get a valid height.
	VerbIcon& cancelVerbIcon = gVerbManager.GetVerbIcon("CANCEL");
	mButtonHolder->SetSizeDelta(cancelVerbIcon.GetWidth(), cancelVerbIcon.GetWidth());
	
	// Hide by default.
	Hide();
}

struct SortActions
{
    bool operator()(const Action* a, const Action* b)
    {
        // First off, if this is INSPECT or INSPECT_UNDO, it should always come first.
        if(StringUtil::EqualsIgnoreCase(a->verb, "INSPECT") || StringUtil::EqualsIgnoreCase(a->verb, "INSPECT_UNDO"))
        {
            return true;
        }
        if(StringUtil::EqualsIgnoreCase(b->verb, "INSPECT") || StringUtil::EqualsIgnoreCase(b->verb, "INSPECT_UNDO"))
        {
            return false;
        }

        // Remaining ones can just be sorted by verb.
        return a->verb < b->verb;
    }
};

void ActionBar::Show(const std::string& noun, VerbType verbType, std::vector<const Action*> actions, std::function<void(const Action*)> executeCallback, std::function<void()> cancelCallback)
{
	// Hide if not already hidden (make sure buttons are freed).
	Hide();
	
	// If we don't have any actions, don't need to do anything!
    std::sort(actions.begin(), actions.end(), SortActions());

    // Save cancel callback.
    mCancelCallback = cancelCallback;
	
	// Iterate over all desired actions and show a button for it.
	bool inventoryShowing = gInventoryManager.IsInventoryShowing();
	int buttonIndex = 0;
	for(size_t i = 0; i < actions.size(); ++i)
	{
		// Kind of a HACK, but it does the trick...if inventory is showing, hide all PICKUP verbs.
		// You can't PICKUP something you already have. And the PICKUP icon is used in inventory to represent "make active".
		if(inventoryShowing && StringUtil::EqualsIgnoreCase(actions[i]->verb, "PICKUP"))
		{
			continue;
		}
		
		// Add button with appropriate icon.
		UIButton* actionButton = nullptr;
		if(verbType == VerbType::Normal)
		{
			VerbIcon& buttonIcon = gVerbManager.GetVerbIcon(actions[i]->verb);
			actionButton = AddButton(buttonIndex, buttonIcon, actions[i]->verb);
		}
		else if(verbType == VerbType::Topic)
		{
			VerbIcon& buttonIcon = gVerbManager.GetTopicIcon(actions[i]->verb);
			actionButton = AddButton(buttonIndex, buttonIcon, actions[i]->verb);
		}
		if(actionButton == nullptr) { continue; }
		++buttonIndex;
		
		// Set up button callback to execute the action.
		const Action* action = actions[i];
		actionButton->SetPressCallback([this, action, executeCallback](UIButton* button) {
			// Hide action bar on button press.
			this->Hide();
			
			// If callback was provided for press, pass handling the press off to that.
			// If no callback was provided, simply play the action!
			if(executeCallback != nullptr)
			{
				executeCallback(action);
			}
			else
			{
				gActionManager.ExecuteAction(action);
			}
		});
	}
	
	// Show inventory item button IF this is a normal action bar (not topic chooser).
	if(verbType == VerbType::Normal)
	{
		// Get active inventory item for current ego.
		std::string activeItemName = gInventoryManager.GetActiveInventoryItem();
		
		// Show inventory button if there's an active inventory item AND it is not the object we're interacting with.
		// In other words, don't allow using an object on itself!
		mHasInventoryItemButton = !activeItemName.empty() && !StringUtil::EqualsIgnoreCase(activeItemName, noun);
		if(mHasInventoryItemButton)
		{
            // Usually, the name of the inventory item correlates EXACTLY to the verb used for that item.
            // However, this isn't the case in a few instances. This seems like a developer error.
            // I don't see any place where a mapping is defined, so I think it must be hardcoded (like this).
            std::string invItemVerb = activeItemName;
            if(StringUtil::EqualsIgnoreCase(activeItemName, "FINGERPRINT_KIT_GRACES"))
            {
                invItemVerb = "FINGERPRINT_KIT";
            }
            else if(StringUtil::EqualsIgnoreCase(activeItemName, "PREPARATION_H_TUBE"))
            {
                invItemVerb = "PREPARATION_H";
            }

            // Create a button for this inventory item.
			VerbIcon& invVerbIcon = gVerbManager.GetInventoryIcon(invItemVerb);
			UIButton* invButton = AddButton(buttonIndex, invVerbIcon, "INV");
			++buttonIndex;
			
			// Create callback for inventory button press.
            const Action* invAction = gActionManager.GetAction(noun, invItemVerb);
			invButton->SetPressCallback([this, invAction, executeCallback](UIButton* button) {
				// Hide action bar on button press.
				this->Hide();
				
				// Execute the action, which will likely run some SheepScript.
                if(executeCallback != nullptr)
                {
                    executeCallback(invAction);
                }
                else
                {
                    gActionManager.ExecuteAction(invAction);
                }
			});
		}
	}
	
	// Always put cancel button on the end.
    if(mAllowCancel)
    {
        VerbIcon& cancelVerbIcon = gVerbManager.GetVerbIcon("CANCEL");
        UIButton* cancelButton = AddButton(buttonIndex, cancelVerbIcon, "CANCEL");

        // Pressing cancel button hides the bar, but it also requires an extra step. So its got its own callback.
        cancelButton->SetPressCallback([this](UIButton* button){
            OnCancelButtonPressed();
        });
    }
	
	// Refresh layout after adding all buttons to position everything correctly.
	RefreshButtonLayout();
	
	// Position action bar at pointer.
	// This also makes sure it doesn't go offscreen.
	CenterOnPointer();
    
	// It's showing now!
	mButtonHolder->GetOwner()->SetActive(true);
    
    // Scene blocker does receive input.
    mSceneBlockerButton->SetReceivesInput(true);
}

void ActionBar::Hide()
{
	// Disable all buttons and put them on the free stack.
	for(auto& button : mButtons)
	{
		button.button->SetEnabled(false);
		mFreeButtons.push(button.button);
	}
	mButtons.clear();
	
	// Button holder inactive = no children show.
	mButtonHolder->GetOwner()->SetActive(false);
    
    // Scene blocker no longer receives input.
    mSceneBlockerButton->SetReceivesInput(false);
}

bool ActionBar::IsShowing() const
{
	return mButtonHolder->IsActiveAndEnabled();
}

bool ActionBar::HasVerb(const std::string& verb) const
{
    for(auto& button : mButtons)
    {
        if(StringUtil::EqualsIgnoreCase(button.verb, verb))
        {
            return true;
        }
    }
    return false;
}

void ActionBar::AddVerbToFront(const std::string& verb, std::function<void()> callback)
{
	// Add button at index 0.
	VerbIcon& icon = gVerbManager.GetVerbIcon(verb);
	UIButton* button = AddButton(0, icon, verb);
	button->SetPressCallback([this, callback](UIButton* button) {
		this->Hide();
		callback();
	});
	
	// Refresh button positions and move bar to pointer.
	RefreshButtonLayout();
	CenterOnPointer();
}

void ActionBar::AddVerbToBack(const std::string& verb, std::function<void()> callback)
{
	VerbIcon& icon = gVerbManager.GetVerbIcon(verb);
	
	// Action bar order is always [INSPECT][VERBS][INV_ITEM][CANCEL]
	// So, skip 1 for cancel button, and maybe skip another one if inventory item is shown.
	int skipCount = 1;
	if(mHasInventoryItemButton)
	{
		skipCount = 2;
	}
	
	// Add button with callback at index.
	UIButton* button = AddButton(static_cast<int>(mButtons.size() - skipCount), icon, verb);
	button->SetPressCallback([this, callback](UIButton* button) {
		this->Hide();
		callback();
	});
	
	// Refresh button positions and move bar to pointer.
	RefreshButtonLayout();
	CenterOnPointer();
}

void ActionBar::SetVerbEnabled(const std::string& verb, bool enabled)
{
    for(auto& button : mButtons)
    {
        if(StringUtil::EqualsIgnoreCase(button.verb, verb))
        {
            button.button->SetCanInteract(enabled);
        }
    }
}

void ActionBar::OnUpdate(float deltaTime)
{
	if(IsShowing()) 
	{
        // Most keyboard input counts as a cancel action, unless some text input is active (like debug window).
        if(!gInputManager.IsTextInput() && mAllowDismiss)
        {
            // Any key press EXCEPT ~ counts as a cancel action.
            // This logic technically blocks any other cancel action due to key presses WHILE ~ is pressed but...close enough.
            if(gInputManager.IsAnyKeyLeadingEdge() && !gInputManager.IsKeyPressed(SDL_SCANCODE_GRAVE))
            {
                OnCancelButtonPressed();
            }
        }
	}
}

UIButton* ActionBar::AddButton(int index, const VerbIcon& buttonIcon, const std::string& verb)
{
	// Reuse a free button or create a new one.
	UIButton* button = nullptr;
	if(mFreeButtons.size() > 0)
	{
		button = mFreeButtons.top();
		mFreeButtons.pop();
		
		// Make sure button is enabled.
		button->SetEnabled(true);
        button->SetCanInteract(true);
		
		// Make sure any old callbacks are no longer set (since we recycle the buttons).
		button->SetPressCallback(nullptr);
	}
	else
	{
		Actor* buttonActor = new Actor(TransformType::RectTransform);
		buttonActor->GetTransform()->SetParent(mButtonHolder);
		button = buttonActor->AddComponent<UIButton>();
	}
	
	// Put into buttons array at desired position.
    mButtons.insert(mButtons.begin() + index, { verb, button });
	
	// Set size correctly.
	button->GetRectTransform()->SetSizeDelta(buttonIcon.GetWidth(), buttonIcon.GetWidth());
	
	// Show correct icon on button.
	button->SetUpTexture(buttonIcon.upTexture);
	button->SetDownTexture(buttonIcon.downTexture);
	button->SetHoverTexture(buttonIcon.hoverTexture);
	button->SetDisabledTexture(buttonIcon.disableTexture);
	return button;
}

void ActionBar::RefreshButtonLayout()
{
	// Iterate over all buttons, positioning each one right after the previous one.
	float xPos = 0.0f;
	for(auto& button : mButtons)
	{		
		// Position correctly, relative to previous buttons.
		RectTransform* buttonRT = button.button->GetRectTransform();
		buttonRT->SetAnchor(Vector2::Zero);
		buttonRT->SetPivot(0.0f, 0.0f);
		buttonRT->SetAnchoredPosition(Vector2(xPos, 0.0f));
		
		xPos += buttonRT->GetSize().x;
	}
	
	// Update the button holder to match the size of all buttons.
	mButtonHolder->SetSizeDeltaX(xPos);
}

void ActionBar::CenterOnPointer()
{
	// Position action bar at mouse position.
	mButtonHolder->SetAnchoredPosition(gInputManager.GetMousePosition());
	
	// Keep inside the screen.
    mButtonHolder->MoveInsideRect(Window::GetRect());
}

void ActionBar::OnCancelButtonPressed()
{
    Hide();

    if(mCancelCallback != nullptr)
    {
        mCancelCallback();
        mCancelCallback = nullptr;
    }
}
