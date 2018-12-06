//
// ActionBar.cpp
//
// Clark Kromenaker
//
#include "ActionBar.h"

#include "UICanvas.h"

ActionBar::ActionBar()
{
	// Create canvas, to contain the UI components.
	mCanvas = AddComponent<UICanvas>();
	
	
}

void ActionBar::UpdateInternal(float deltaTime)
{
	
}
