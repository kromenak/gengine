//
// UICanvas.h
//
// Clark Kromenaker
//
// A collection of UI components that are logically related and
// updated/drawn/managed together.
//
// A UI "canvas" could represent one distinct UI screen or dialog.
// E.g. a Main Menu, an Options Screen, a Popup, a Tooltip.
//
#pragma once
#include "UIWidget.h"

class UICanvas : public UIWidget
{
	TYPE_DECL_CHILD();
public:
	static const std::vector<UICanvas*>& GetCanvases() { return sCanvases; }
	static void UpdateInput();
	static bool DidWidgetEatInput() { return sMouseOverWidget != nullptr; }
	
	UICanvas(Actor* owner);
	~UICanvas();
	
	void Render() override;
	
	void AddWidget(UIWidget* widget);
	void RemoveWidget(UIWidget* widget);
	void RemoveAllWidgets() { mWidgets.clear(); }
	
private:
	// An array of all canvases that currently exist.
	static std::vector<UICanvas*> sCanvases;
	
	// At any time, the mouse can be over exactly one widget.
	// (at least, unless we add multi-pointer support...shudders)
	static UIWidget* sMouseOverWidget;
	
	// All widgets on this canvas.
	std::vector<UIWidget*> mWidgets;
};
