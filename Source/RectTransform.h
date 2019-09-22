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

#include "Rect.h"
#include "Vector2.h"

class RectTransform : public Transform
{
	TYPE_DECL_CHILD();
public:
	RectTransform(Actor* owner);
	
	void SetSizeDelta(float x, float y);
	void SetSizeDelta(Vector2 size);
	Vector2 GetSize() const;
	
	void SetPivot(float x, float y);
	void SetPivot(Vector2 pivot);
	Vector2 GetPivot() const { return mPivot; }
	
	void SetAnchor(const Vector2& anchor);
	void SetAnchorMin(const Vector2& anchorMin);
	void SetAnchorMax(const Vector2& anchorMax);
	Vector2 GetAnchorMin() const { return mAnchorMin; }
	Vector2 GetAnchorMax() const { return mAnchorMax; }
	
	void SetAnchoredPosition(float x, float y);
	void SetAnchoredPosition(const Vector2& anchoredPosition);
	Vector2 GetAnchoredPosition() const { return mAnchoredPosition; }
	
	void SetAnchorOffsetsX(float x) { }
	void SetAnchorOffsetsY(float y) { }
	void SetAnchorOffsets(float x, float y) { }
	
	//SetLeft
	//SetRight
	//SetTop
	//SetBottom
	
	Rect GetRect() const;
	Rect GetScreenRect();
	
	Matrix4 GetLocalRectOffset() { return Matrix4::MakeTranslate(GetRect().GetMin()); }
	
protected:
	Vector3 GetLocalPosition() override;
	void UpdateInternal(float deltaTime) override;
	
private:
	// Identifies an anchor rect that is used to position & size this RectTransform relative to its parent's Rect.
	// If values are equal, the anchor is a single point (anchored position defines offset from that point).
	// If values aren't equal, the anchor is 2 or 4 points. In this case, size is controlled by min/max and anchor offsets. Anchored position is an offset from the midpoint.
	Vector2 mAnchorMin = Vector2(0.5f, 0.5f); // Equates to left/bottom
	Vector2 mAnchorMax = Vector2(0.5f, 0.5f); // Equates to right/top
	
	// The pivot identifies this transform's origin. Another way to look at it is that it defines the position of the Rect relative to the local origin.
	// Pivot is a normalized value (0-1). (0, 0) is bottom-left, (1, 1) is top-right, (0.5, 0.5) is center.
	// Values beyond 0-1 are also possible - this will put the pivot outside the rect by some amount (every "1" unit equates to the width/height of the rect).
	Vector2 mPivot = Vector2(0.5f, 0.5f);
	
	// When anchor axis has equal values, this defines an offset from that value.
	// When anchor axis has unequal values, this defines an offset from the midpoint of the two anchors.
	Vector2 mAnchoredPosition = Vector2::Zero;
	
	// The size of the rect area (width/height).
	// This can usually be set as desired, but it may be overridden due to anchor settings.
	Vector2 mSizeDelta = Vector2(100.0f, 100.0f);
};
