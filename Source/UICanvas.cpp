//
// UICanvas.cpp
//
// Clark Kromenaker
//
#include "UICanvas.h"

#include "Services.h"

TYPE_DEF_CHILD(UIWidget, UICanvas);

std::vector<UICanvas*> UICanvas::sCanvases;
UIWidget* UICanvas::sMouseOverWidget = nullptr;

/*static*/ void UICanvas::UpdateInput()
{
	// Iterate canvases back-to-front. Since canvases at end of list are rendered last,
	// they should be the first to receive input events.
	for(int i = static_cast<int>(sCanvases.size() - 1); i >= 0; --i)
	{
		// Ignore canvases that aren't active & enabled.
		UICanvas* canvas = sCanvases[i];
		if(!canvas->IsActiveAndEnabled()) { continue; }
		
		// Same thing for widgets on each canvas - iterate back-to-front.
		// Same reason as above: last widgets render on top of everything, so they should receive input events first.
		for(int j = static_cast<int>(canvas->mWidgets.size() - 1); j >= 0; --j)
		{
			// Ignore widgets that aren't active and enabled.
			UIWidget* widget = canvas->mWidgets[j];
			if(!widget->IsActiveAndEnabled()) { continue; }
			
            // Widgets that are not recieving inputs should be ignored.
            if(!widget->ReceivesInput()) { continue; }
            
			// See whether the pointer is over this widget.
			RectTransform* widgetRT = widget->GetRectTransform();
			if(widgetRT->GetWorldRect().Contains(Services::GetInput()->GetMousePosition()))
			{
				// Pointer is over this widget, but it is not the "over" widget yet!
				if(sMouseOverWidget != widget)
				{
					// Any previous "over" widget no longer has the pointer over it.
					if(sMouseOverWidget != nullptr)
					{
						sMouseOverWidget->OnPointerExit();
					}
					
					// All hail the new widget with the pointer over it.
					sMouseOverWidget = widget;
					sMouseOverWidget->OnPointerEnter();
				}
				
				// If pointer goes down or up this frame, notify whatever widget the pointer is currently over.
				if(sMouseOverWidget != nullptr)
				{
					if(Services::GetInput()->IsMouseButtonDown(InputManager::MouseButton::Left))
					{
						sMouseOverWidget->OnPointerDown();
					}
					else if(Services::GetInput()->IsMouseButtonUp(InputManager::MouseButton::Left))
					{
						sMouseOverWidget->OnPointerUp();
					}
				}
				
				// Inputs on further widgets or canvases are not possible, since this widget "ate" the input.
				//TODO: We could add some sort of "ignores input events" flag for certain widgets, if needed.
				return;
			}
		}
	}
	
	// If we get here, the pointer is not over ANY widget.
	// So, if we had a saved widget, the pointer is clearly no longer over it either...
	if(sMouseOverWidget != nullptr)
	{
		sMouseOverWidget->OnPointerExit();
		sMouseOverWidget = nullptr;
	}
}

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
