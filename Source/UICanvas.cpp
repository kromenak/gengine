//
// UICanvas.cpp
//
// Clark Kromenaker
//
#include "UICanvas.h"

TYPE_DEF_CHILD(UIWidget, UICanvas);

std::vector<UICanvas*> UICanvas::sCanvases;

UICanvas::UICanvas(Actor* owner) : UIWidget(owner)
{
	sCanvases.push_back(this);
}

UICanvas::~UICanvas()
{
	auto it = std::find(sCanvases.begin(), sCanvases.end(), this);
	if(it != sCanvases.end())
	{
		sCanvases.erase(it);
	}
}

void UICanvas::Render()
{
	if(IsActiveAndEnabled())
	{
		for(auto& widget : mWidgets)
		{
			if(widget->IsActiveAndEnabled())
			{
				widget->Render();
			}
		}
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
