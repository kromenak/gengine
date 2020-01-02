//
// UIWidget.cpp
//
// Clark Kromenaker
//
// Base class for any UI element, called a "widget".
// This class is abstract.
//
#pragma once
#include "Component.h"

#include "RectTransform.h"

class UIWidget : public Component
{
    TYPE_DECL_CHILD();
public:
    UIWidget(Actor* actor);
    virtual ~UIWidget();
    
	virtual void Render() = 0;
	
	virtual void OnPointerEnter() { }
	virtual void OnPointerExit() { }
	
	virtual void OnPointerDown() { }
	virtual void OnPointerUp() { }
	
	RectTransform* GetRectTransform() const { return mRectTransform; }
	
protected:
	// UI widgets usually rely heavily on a RectTransform.
	// So, we cache it here.
	RectTransform* mRectTransform = nullptr;
	
	Matrix4 GetWorldTransformWithSizeForRendering();
};
