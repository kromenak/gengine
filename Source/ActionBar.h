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

class UICanvas;

class ActionBar : public Actor
{
public:
	ActionBar();
	
protected:
	void UpdateInternal(float deltaTime) override;
	
private:
	UICanvas* mCanvas;
};
