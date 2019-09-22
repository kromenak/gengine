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

void RectTransform::SetSizeDelta(Vector2 size)
{
	mSizeDelta = size;
	SetDirty();
}

Vector2 RectTransform::GetSize() const
{
	return GetRect().GetSize();
}

void RectTransform::SetPivot(float x, float y)
{
	SetPivot(Vector2(x, y));
}

void RectTransform::SetPivot(Vector2 pivot)
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
		parentRect = Rect(0.0f, 0.0f, windowSize.GetX(), windowSize.GetY());
	}
	else
	{
		RectTransform* parent = static_cast<RectTransform*>(mParent);
		parentRect = parent->GetRect();
	}
	
	return RectUtil::CalcLocalRect(parentRect, mAnchorMin, mAnchorMax, mSizeDelta, mPivot);
}

Rect RectTransform::GetScreenRect()
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

Vector3 RectTransform::GetLocalPosition()
{
	Rect parentRect;
	Vector2 parentPivot;
	if(mParent == nullptr || !mParent->IsTypeOf(RectTransform::GetType()))
	{
		Vector2 windowSize = Services::GetRenderer()->GetWindowSize();
		parentRect = Rect(0.0f, 0.0f, windowSize.GetX(), windowSize.GetY());
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
	mLocalPosition.SetX(localPos.GetX());
	mLocalPosition.SetY(localPos.GetY());
	return mLocalPosition;
}

void RectTransform::UpdateInternal(float deltaTime)
{
	// For debugging: visualize min/max of the rect calculated for this RectTransform.
	if(Debug::RenderRectTransformRects())
	{
		Rect screenRect = GetScreenRect();
		Vector3 min = Services::GetRenderer()->GetCamera()->ScreenToWorldPoint(screenRect.GetMin(), 0.0f);
		Vector3 max = Services::GetRenderer()->GetCamera()->ScreenToWorldPoint(screenRect.GetMax(), 0.0f);
		Debug::DrawLine(min, max, Color32::Cyan);
	}
}
