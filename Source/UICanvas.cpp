//
// UICanvas.cpp
//
// Clark Kromenaker
//
#include "UICanvas.h"

#include "Actor.h"

TYPE_DEF_CHILD(UIWidget, UICanvas);

UICanvas::UICanvas(Actor* owner) : UIWidget(owner)
{
	
}

void UICanvas::Render()
{
	for(auto& widget : mWidgets)
	{
		widget->Render();
	}
}

void UICanvas::AddWidget(UIWidget* widget)
{
	mWidgets.push_back(widget);
}

void UICanvas::RemoveWidget(UIWidget* widget)
{
	auto it = std::find(mWidgets.begin(), mWidgets.end(), widget);
	if(it != mWidgets.end())
	{
		mWidgets.erase(it);
	}
}
