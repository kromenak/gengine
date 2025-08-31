#include "ActionBar.h"

#include <algorithm>

#include "GK3UI.h"
#include "InputManager.h"
#include "InventoryManager.h"
#include "StringUtil.h"
#include "UIButton.h"
#include "UIUtil.h"
#include "VerbManager.h"
#include "Window.h"

namespace
{
    struct SortActions
    {
        bool operator()(const Action* a, const Action* b)
        {
            // First off, if a or b is INSPECT or INSPECT_UNDO, it should always come first.
            bool aIsInspect = StringUtil::EqualsIgnoreCase(a->verb, "INSPECT") || StringUtil::EqualsIgnoreCase(a->verb, "INSPECT_UNDO");
            bool bIsInspect = StringUtil::EqualsIgnoreCase(b->verb, "INSPECT") || StringUtil::EqualsIgnoreCase(b->verb, "INSPECT_UNDO");
            if(aIsInspect && !bIsInspect)
            {
                return true;
            }
            if(!aIsInspect && bIsInspect)
            {
                return false;
            }

            // Remaining ones can just be sorted by verb.
            return a->verb < b->verb;
        }
    };
}

ActionBar::ActionBar() : Actor("ActionBar", TransformType::RectTransform)
{
    // Create canvas, to contain the UI components.
    UI::AddCanvas(this, 11);

    // The background of the action bar consists of a fullscreen clickable button area.
    // This stops interaction with the scene while action bar is visible.
    // It also allows clicking outside the bar to cancel it.
    mSceneBlockerButton = AddComponent<UIButton>();
    mSceneBlockerButton->SetPressCallback([this](UIButton* button){
        if(mAllowDismiss)
        {
            OnCancelButtonPressed();
        }
    });

    // Create button holder - it holds the buttons and we move it around the screen.
    // Since we will set the action bar's position based on mouse position, set the anchor to the lower-left corner.
    Actor* buttonHolderActor = new Actor("ButtonHolder", TransformType::RectTransform);
    mButtonHolder = buttonHolderActor->GetComponent<RectTransform>();
    mButtonHolder->SetParent(GetTransform());
    mButtonHolder->SetAnchorMin(Vector2::Zero);
    mButtonHolder->SetAnchorMax(Vector2::Zero);
    mButtonHolder->SetPivot(0.5f, 0.5f);

    // To have button holder appear in correct spot, we need the holder to be the right height.
    // So, just use one of the buttons to get a valid height.
    VerbIcon& cancelVerbIcon = gVerbManager.GetVerbIcon("CANCEL");
    mButtonHolder->SetSizeDelta(cancelVerbIcon.GetWidth(), cancelVerbIcon.GetWidth());

    // Hide by default.
    Hide(false);
}

void ActionBar::Show(const std::string& noun, VerbType verbType, std::vector<const Action*> actions,
                     std::function<void(const Action*)> executeCallback, std::function<void()> cancelCallback,
                     bool centerOnPointer)
{
    // Hide if not already hidden (make sure buttons are freed).
    Hide(false);

    // Sort actions based on some criteria, so they appear in a deterministic order when the bar is shown.
    std::sort(actions.begin(), actions.end(), SortActions());

    // Save cancel callback.
    mCancelCallback = cancelCallback;

    // Iterate over all desired actions and show a button for it.
    bool inventoryShowing = gInventoryManager.IsInventoryShowing();
    int buttonIndex = 0;
    const Action* cancelAction = nullptr;
    for(size_t i = 0; i < actions.size(); ++i)
    {
        // Kind of a HACK, but it does the trick...if inventory is showing, hide all PICKUP verbs.
        // You can't PICKUP something you already have. And the PICKUP icon is used in inventory to represent "make active".
        if(inventoryShowing && StringUtil::EqualsIgnoreCase(actions[i]->verb, "PICKUP"))
        {
            continue;
        }

        // In rare cases, the NVC specifies a custom CANCEL action.
        // In that case, save it so we can add it to the bar last (cancel should always be the last action in the bar).
        if(StringUtil::EqualsIgnoreCase(actions[i]->verb, "CANCEL"))
        {
            cancelAction = actions[i];
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
        actionButton->SetPressCallback([this, action, executeCallback](UIButton* button){
            // Hide action bar on button press.
            this->Hide(false);

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
        actionButton->SetTooltipText("v_" + actions[i]->verb);
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
            invButton->SetPressCallback([this, invAction, executeCallback](UIButton* button){
                // Hide action bar on button press.
                this->Hide(false);

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
            invButton->SetTooltipText("v_" + invItemVerb);
        }
    }

    // Always put cancel button on the end.
    if(mAllowCancel)
    {
        VerbIcon& cancelVerbIcon = gVerbManager.GetVerbIcon("CANCEL");
        UIButton* cancelButton = AddButton(buttonIndex, cancelVerbIcon, "CANCEL");

        // Pressing cancel button hides the bar, but it also requires an extra step. So its got its own callback.
        cancelButton->SetPressCallback([this, cancelAction](UIButton* button){
            if(cancelAction != nullptr)
            {
                gActionManager.ExecuteAction(cancelAction);
            }
            else
            {
                OnCancelButtonPressed();
            }
        });
        cancelButton->SetTooltipText("v_t_cancel");
    }

    // Refresh layout after adding all buttons to position everything correctly.
    RefreshButtonLayout();

    // If desired, center the action bar under the pointer's current position.
    // This typically only happens when the action bar is shown due to a click input.
    if(centerOnPointer)
    {
        CenterOnPointer();
    }
    else
    {
        // Rarely, the action bar may show, be told NOT to center on pointer, and still be in it's default lower-left corner position.
        // In this case, force the bar to show centered on-screen, which looks nicer for a default position.
        // (One known case: launch game, load save game on 303P timeblock screen. The first action bar shown doesn't center on pointer.)
        if(mButtonHolder->GetAnchoredPosition() == Vector2::Zero)
        {
            mButtonHolder->SetAnchoredPosition(Window::GetSize() * 0.5f);
        }

        // If bar shows in same spot as previous, it may have more buttons and can extend off-screen.
        // This ensures we stay on screen at least!
        KeepOnScreen();
    }

    // It's showing now!
    mButtonHolder->GetOwner()->SetActive(true);

    // Scene blocker does receive input.
    mSceneBlockerButton->SetReceivesInput(true);
}

void ActionBar::Hide(bool cancel)
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

    // If this was hidden due to a cancel, call the cancel callback.
    if(cancel && mCancelCallback != nullptr)
    {
        mCancelCallback();
        mCancelCallback = nullptr;
    }
}

bool ActionBar::IsShowing() const
{
    return mButtonHolder->IsActiveAndEnabled();
}

bool ActionBar::HasVerb(const std::string& verb) const
{
    return GetVerbIndex(verb) >= 0;
}

int ActionBar::GetVerbIndex(const std::string& verb) const
{
    for(int i = 0; i < mButtons.size(); ++i)
    {
        if(StringUtil::EqualsIgnoreCase(mButtons[i].verb, verb))
        {
            return i;
        }
    }
    return -1;
}

void ActionBar::AddVerbToFront(const std::string& verb, const std::function<void()>& callback)
{
    AddVerbAtIndex(verb, 0, callback);
}

void ActionBar::AddVerbToBack(const std::string& verb, const std::function<void()>& callback)
{
    // Action bar order is always [INSPECT][VERBS][INV_ITEM][CANCEL]
    // So, skip 1 for cancel button, and maybe skip another one if inventory item is shown.
    int skipCount = mHasInventoryItemButton ? 2 : 1;

    // Put verb at desired index.
    AddVerbAtIndex(verb, static_cast<int>(mButtons.size() - skipCount), callback);
}

void ActionBar::AddVerbAtIndex(const std::string& verb, int index, const std::function<void()>& callback)
{
    VerbIcon& icon = gVerbManager.GetVerbIcon(verb);

    // Add button with callback at index.
    UIButton* button = AddButton(index, icon, verb);
    button->SetPressCallback([this, callback](UIButton* button){
        this->Hide(false);
        callback();
    });

    // Set tooltip text.
    button->SetTooltipText("v_" + verb);

    // Refresh button positions and move bar to pointer.
    RefreshButtonLayout();
    KeepOnScreen();
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

void ActionBar::RemoveVerb(const std::string& verb)
{
    int index = GetVerbIndex(verb);
    if(index >= 0)
    {
        mFreeButtons.push(mButtons[index].button);
        mButtons[index].button->SetEnabled(false);

        mButtons.erase(mButtons.begin() + index);
        RefreshButtonLayout();
        KeepOnScreen();
    }
}

void ActionBar::Dismiss()
{
    // Dismiss is similar to hide/cancel, but it's disallowed in some cases.
    if(mAllowDismiss)
    {
        OnCancelButtonPressed();
    }
}

void ActionBar::OnUpdate(float deltaTime)
{
    if(IsShowing())
    {
        // Any key press causes the action bar to be dismissed.
        // ESC as well, BUT that scenario is handled in GameCamera, since the logic is more complex there.
        if(gGK3UI.IsAnyKeyPressedOutsideTextInputAndConsole() && !gInputManager.IsKeyLeadingEdge(SDL_SCANCODE_ESCAPE))
        {
            Dismiss();
        }

        // If an action is playing, and the action bar is dismissable, hide the bar without calling cancel callback.
        // This can happen if action bar is up, but then a timer or other background script causes an action/cutscene to execute.
        if(gActionManager.IsActionPlaying() && mAllowDismiss)
        {
            Hide(false);
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
    button->GetOwner()->SetName(verb);

    // Put into buttons array at desired position.
    // Note that "one past the last index" is a valid index here - that'll put the thing on the end of the list.
    index = Math::Clamp<int>(index, 0, mButtons.size());
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
    mButtonHolder->SetAnchoredPosition(GetTransform()->GetWorldToLocalMatrix().TransformPoint(gInputManager.GetMousePosition()));

    // Make sure it's still on-screen.
    KeepOnScreen();
}

void ActionBar::KeepOnScreen()
{
    // Moving the button holder may put it offscreen, if the pointer is near the screen edge.
    // So, move it back on-screen in that case.
    mButtonHolder->MoveInsideRect(Window::GetRect());
}

void ActionBar::OnCancelButtonPressed()
{
    Hide(true);
}
