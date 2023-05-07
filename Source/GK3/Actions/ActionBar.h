//
// Clark Kromenaker
//
// A UI that appears when the the player needs to choose an action to
// perform, either during a conversation (picking a topic) or when exploring
// the environment (examining, opening, taking, etc).
//
#pragma once
#include "Actor.h"

#include <functional>
#include <stack>
#include <string>
#include <vector>

#include "ActionManager.h"
#include "NVC.h"

struct VerbIcon;
class RectTransform;
class UIButton;

class ActionBar : public Actor
{
public:
	ActionBar();
	
	void Show(const std::string& noun, VerbType verbType, std::vector<const Action*> actions, std::function<void(const Action*)> executeCallback, std::function<void()> cancelCallback);
	void Hide();
	
	bool IsShowing() const;

    bool HasVerb(const std::string& verb) const;
	void AddVerbToFront(const std::string& verb, std::function<void()> callback);
	void AddVerbToBack(const std::string& verb, std::function<void()> callback);
    void SetVerbEnabled(const std::string& verb, bool enabled);

    void SetAllowCancel(bool allowCancel) { mAllowCancel = allowCancel; }
    void SetAllowDismiss(bool allowDismiss) { mAllowDismiss = allowDismiss; }
	
protected:
	void OnUpdate(float deltaTime) override;
	
private:
    // If true, a cancel button is displayed. Pressing it will hide the action bar.
    // This may be disabled in situations where the bar is not allowed to be canceled.
    bool mAllowCancel = true;

    // If true, the bar can be dismissed by clicking outside of it, or pressing escape.
    // This may be disabled when the bar can't be dismissed (forced to make a choice).
    bool mAllowDismiss = true;
    
    // A large clickable area behind the action bar that stops scene interaction while visible.
    UIButton* mSceneBlockerButton = nullptr;
	
	// A transform that is parent for all buttons.
	RectTransform* mButtonHolder = nullptr;
    
	// Buttons that are created, but not shown. These can be reused when needed.
	std::stack<UIButton*> mFreeButtons;
	
	// The buttons currently showing on the action bar, in order left-to-right.
    struct ActionButton
    {
        std::string verb;
        UIButton* button = nullptr;
    };
	std::vector<ActionButton> mButtons;
	
	// Currently showing inventory button?
	bool mHasInventoryItemButton = false;
	
	// Callback that executes if the action bar is canceled.
    // This IS NOT the same as hiding! Cancel only happens if cancel button is pressed, click outside action bar, or press backspace.
	std::function<void()> mCancelCallback = nullptr;
	
	UIButton* AddButton(int index, const VerbIcon& buttonIcon, const std::string& verb);
	void RefreshButtonLayout();
	void CenterOnPointer();

    void OnCancelButtonPressed();
};
