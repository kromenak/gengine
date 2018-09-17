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

#include "Matrix4.h"
#include "Vector2.h"

class UIWidget : public Component
{
    TYPE_DECL_CHILD();
public:
    UIWidget(Actor* actor);
    ~UIWidget();
    
	virtual void Render() = 0;
    
protected:
	// The rectangular size of the widget.
    Vector2 mSize;
	
	// The pivot point of the widget.
	// (0, 0) is top-left, (1, 1) is bottom-right, (0.5, 0.5) is center.
	Vector2 mPivot;
	
	// The anchor is a normalized point on a parent that this widget is positioned relative to.
	// (0, 0) is top-left corner, (1, 1) is bottom-right, (0.5, 0.5) is the center.
	Vector2 mAnchor;
	
	Matrix4 GetUIWorldTransformMatrix();
};
