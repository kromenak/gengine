//
// RectTransform.cpp
//
// Clark Kromenaker
//
#include "RectTransform.h"

#include "Camera.h"
#include "Debug.h"
#include "RectUtil.h"
#include "Services.h"

TYPE_DEF_CHILD(Transform, RectTransform);

RectTransform::RectTransform(Actor* owner) : Transform(owner)
{
	
}

void RectTransform::SetSizeDelta(float x, float y)
{
	SetSizeDelta(Vector2(x, y));
}

void RectTransform::SetSizeDelta(const Vector2& size)
{
	mSizeDelta = size;
	SetDirty();
}

void RectTransform::SetSizeDeltaX(float x)
{
	mSizeDelta.x = x;
	SetDirty();
}

void RectTransform::SetSizeDeltaY(float y)
{
	mSizeDelta.y = y;
	SetDirty();
}

void RectTransform::SetPivot(float x, float y)
{
	SetPivot(Vector2(x, y));
}

void RectTransform::SetPivot(const Vector2& pivot)
{
	mPivot = pivot;
	SetDirty();
}

void RectTransform::SetAnchor(const Vector2& anchor)
{
	 mAnchorMin = anchor;
	 mAnchorMax = anchor;
	 SetDirty();
}

void RectTransform::SetAnchorMin(const Vector2& anchorMin)
{
	mAnchorMin = anchorMin;
	SetDirty();
}

void RectTransform::SetAnchorMax(const Vector2& anchorMax)
{
	mAnchorMax = anchorMax;
	SetDirty();
}

void RectTransform::SetAnchoredPosition(float x, float y)
{
	SetAnchoredPosition(Vector2(x, y));
}

void RectTransform::SetAnchoredPosition(const Vector2& anchoredPosition)
{
	mAnchoredPosition = anchoredPosition;
	SetDirty();
}

Rect RectTransform::GetRect() const
{
	Rect parentRect;
	if(mParent == nullptr || !mParent->IsTypeOf(RectTransform::GetType()))
	{
		Vector2 windowSize = Services::GetRenderer()->GetWindowSize();
		parentRect = Rect(0.0f, 0.0f, windowSize.x, windowSize.y);
	}
	else
	{
		RectTransform* parent = static_cast<RectTransform*>(mParent);
		parentRect = parent->GetRect();
	}
	return RectUtil::CalcLocalRect(parentRect, mAnchorMin, mAnchorMax, mSizeDelta, mPivot);
}

Rect RectTransform::GetWorldRect()
{
	Rect localRect = GetRect();
	
	// "min" and "max" here should correlate to the min and max of the quad used for rendering.
	// Ex: A quad centered on origin should use min of -(mSize / 2) and max of (mSize / 2).
	// Ex: A quad with bottom-left corner at origin should use (0, 0, 0) and mSize respectively.
	// Our UI quad has a min of (0, 0) and a max of (1, 1).
	// TODO: Does this need to be dynamic in some way, or will we always render with bottom-left origin quad?
	Vector2 min = localRect.GetMin(); //Vector3::Zero;
	Vector2 max = localRect.GetMax(); //mSizeDelta;
	
	// Transform those points based on this transform's parents and scale/rotation/translation.
	min = LocalToWorldPoint(min);
	max = LocalToWorldPoint(max);
	
	// Construct and return the rect.
	return Rect(min, max);
}

void RectTransform::CalcLocalPosition()
{
	Rect parentRect;
	Vector2 parentPivot;
	if(mParent == nullptr || !mParent->IsTypeOf(RectTransform::GetType()))
	{
		Vector2 windowSize = Services::GetRenderer()->GetWindowSize();
		parentRect = Rect(0.0f, 0.0f, windowSize.x, windowSize.y);
		parentPivot = Vector2(0.0f, 0.0f);
	}
	else
	{
		RectTransform* parent = static_cast<RectTransform*>(mParent);
		parentRect = parent->GetRect();
		parentPivot = parent->GetPivot();
	}
	
	Vector3 localPos = RectUtil::CalcLocalPosition(parentRect, parentPivot, mAnchorMin, mAnchorMax, mAnchoredPosition, mPivot);
	
	// Update local pos x/y components.
	// Don't overwrite the z-component, which can be set freely.
	mLocalPosition.x = localPos.x;
	mLocalPosition.y = localPos.y;
}

void RectTransform::OnUpdate(float deltaTime)
{
	// For debugging: visualize min/max of the rect calculated for this RectTransform.
	if(Debug::RenderRectTransformRects() || debugVisualizeRect)
	{
        // For UI, "world space" is really "screen space". Confusing, I know.
        // So, get screen rect, convert to world space.
		Rect screenRect = GetWorldRect();
        Vector3 min = Services::GetRenderer()->GetCamera()->ScreenToWorldPoint(screenRect.GetMin(), 0.0f);
        Vector3 max = Services::GetRenderer()->GetCamera()->ScreenToWorldPoint(screenRect.GetMax(), 0.0f);
        
        // Generate corners of the rectangular area in 3D space.
        Vector3 p0 = min;
        Vector3 p1(min.x, max.y, min.z);
        Vector3 p2 = max;
        Vector3 p3(max.x, min.y, max.z);
        
        // Draw lines to create rectangle.
        Debug::DrawLine(p0, p1, Color32::Cyan);
        Debug::DrawLine(p1, p2, Color32::Cyan);
        Debug::DrawLine(p2, p3, Color32::Cyan);
        Debug::DrawLine(p3, p0, Color32::Cyan);
	}
}
