//
// RectTransform.h
//
// Clark Kromenaker
//
// A transform that transforms not just a point in 3D space,
// but a rectangular area. Most useful for UI.
//
#pragma once
#include "Transform.h"

#include "Vector2.h"
#include "Rect.h"

class RectTransform : public Transform
{
	TYPE_DECL_CHILD();
public:
	RectTransform(Actor* owner);
	
	void SetSize(float x, float y) { mSize.SetX(x); mSize.SetY(y); }
	void SetSize(Vector2 size) { mSize = size; }
	Vector2 GetSize() { return mSize; }
	
	Rect GetScreenRect();
	
protected:
	Vector3 GetLocalPosition() override;
	
private:
	// The size of the rect area (width/height).
	Vector2 mSize = Vector2(1.0f, 1.0f);
	
	// The pivot is the spot that this transform rotates around.
	// (0, 0) is top-left, (1, 1) is bottom-right, (0.5, 0.5) is center.
	Vector2 mPivot = Vector2(0.5f, 0.5f);
	
	// Min/max anchors for the rect. These are normalized positions within the parent rect.
	// When these values are equal, we are anchored to a point in the parent rect.
	// When these values are not equal, then the left/right/bottom/top of the rect are anchored to the parent rect based on those values.
	Vector2 mAnchorMin = Vector2(0.5f, 0.5f);
	Vector2 mAnchorMax = Vector2(0.5f, 0.5f);
};
