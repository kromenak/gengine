//
// ActionBar.h
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
#include <vector>

#include "NVC.h"

class ButtonIcon;
class UIButton;
class UICanvas;

class ActionBar : public Actor
{
public:
	ActionBar();
	
	void Show(std::vector<const NVCItem*> actions, std::function<void(const NVCItem*)> executeCallback);
	void Hide();
	
	bool IsShowing() const { return mIsShowing; }
	
protected:
	void UpdateInternal(float deltaTime) override;
	
private:
	// The action bar's canvas, which renders the UI.
	UICanvas* mCanvas = nullptr;
	
	// The buttons created for the action bar. Recycled on each show.
	std::vector<UIButton*> mButtons;
	
	// If true, action bar is visible and waiting for input.
	bool mIsShowing = false;
	
	UIButton* AddButton(int index, float xPos, const ButtonIcon& buttonIcon);
};
