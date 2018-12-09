//
// RectTransform.cpp
//
// Clark Kromenaker
//
#include "RectTransform.h"

#include "Services.h"

TYPE_DEF_CHILD(Transform, RectTransform);

RectTransform::RectTransform(Actor* owner) : Transform(owner)
{
	
}

Rect RectTransform::GetScreenRect()
{
	// Calculate the rect min/max centered around local origin.
	Vector3 min = -(mSize / 2.0f);
	Vector3 max = (mSize / 2.0f);
	
	// Transform those points based on this transform's parents and scale/rotation/translation.
	min = LocalToWorldPoint(min);
	max = LocalToWorldPoint(max);
	
	// Construct the rect.
	return Rect(min.GetX(), min.GetY(), max.GetX() - min.GetX(), max.GetY() - min.GetY());
}

Vector3 RectTransform::GetLocalPosition()
{
	// We need to determine the Rect of our parent.
	// If we have no parent, we'll use the screen width/height.
	Vector2 size;
	if(mParent != nullptr && mParent->IsTypeOf(RectTransform::GetType()))
	{
		RectTransform* rectTransform = static_cast<RectTransform*>(mParent);
		size = rectTransform->GetSize();
	}
	else
	{
		size = Services::GetRenderer()->GetWindowSize();
	}
	
	// Determine our anchor on the parent.
	Vector3 anchor;
	anchor.SetX(size.GetX() * mAnchorMin.GetX());
	anchor.SetY(size.GetY() * mAnchorMin.GetY());
	
	// The actual local position is an offset from the anchor.
	return anchor + mLocalPosition;
}
