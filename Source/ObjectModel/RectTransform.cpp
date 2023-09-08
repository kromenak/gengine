#include "RectTransform.h"

#include "Camera.h"
#include "Debug.h"
#include "RectUtil.h"
#include "Window.h"

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

void RectTransform::SetAnchor(AnchorPreset preset, bool setPivot)
{
    switch(preset)
    {
    case AnchorPreset::TopLeft:
        SetAnchor(Vector2(0.0f, 1.0f));
        if(setPivot) { SetPivot(Vector2(0.0f, 1.0f)); }
        break;

    case AnchorPreset::Top:
        SetAnchor(Vector2(0.5f, 1.0f));
        if(setPivot) { SetPivot(Vector2(0.5f, 1.0f)); }
        break;

    case AnchorPreset::TopStretch:
        SetAnchorMin(Vector2(0.0f, 1.0f));
        SetAnchorMax(Vector2(1.0f, 1.0f));
        if(setPivot) { SetPivot(Vector2(0.5f, 1.0f)); }
        break;

    case AnchorPreset::TopRight:
        SetAnchor(Vector2(1.0f, 1.0f));
        if(setPivot) { SetPivot(Vector2(1.0f, 1.0f)); }
        break;

    case AnchorPreset::Left:
        SetAnchor(Vector2(0.0f, 0.5f));
        if(setPivot) { SetPivot(Vector2(0.0f, 0.5f)); }
        break;

    case AnchorPreset::LeftStretch:
        SetAnchorMin(Vector2(0.0f, 0.0f));
        SetAnchorMax(Vector2(0.0f, 1.0f));
        if(setPivot) { SetPivot(Vector2(0.0f, 0.5f)); }
        break;

    case AnchorPreset::Center:
        SetAnchor(Vector2(0.5f, 0.5f));
        if(setPivot) { SetPivot(Vector2(0.5f, 0.5f)); }
        break;

    case AnchorPreset::CenterStretch:
        SetAnchorMin(Vector2(0.0f, 0.0f));
        SetAnchorMax(Vector2(1.0f, 1.0f));
        if(setPivot) { SetPivot(Vector2(0.5f, 0.5f)); }
        break;

    case AnchorPreset::Right:
        SetAnchor(Vector2(1.0f, 0.5f));
        if(setPivot) { SetPivot(Vector2(1.0f, 0.5f)); }
        break;

    case AnchorPreset::RightStretch:
        SetAnchorMin(Vector2(1.0f, 0.0f));
        SetAnchorMax(Vector2(1.0f, 1.0f));
        if(setPivot) { SetPivot(Vector2(1.0f, 0.5f)); }
        break;

    case AnchorPreset::BottomLeft:
        SetAnchor(Vector2(0.0f, 0.0f));
        if(setPivot) { SetPivot(Vector2(0.0f, 0.0f)); }
        break;

    case AnchorPreset::Bottom:
        SetAnchor(Vector2(0.5f, 0.0f));
        if(setPivot) { SetPivot(Vector2(0.5f, 0.0f)); }
        break;

    case AnchorPreset::BottomStretch:
        SetAnchorMin(Vector2(0.0f, 0.0f));
        SetAnchorMax(Vector2(1.0f, 0.0f));
        if(setPivot) { SetPivot(Vector2(0.5f, 0.0f)); }
        break;

    case AnchorPreset::BottomRight:
        SetAnchor(Vector2(1.0f, 0.0f));
        if(setPivot) { SetPivot(Vector2(1.0f, 0.0f)); }
        break;
    }
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
        Vector2 windowSize = Window::GetSize();
		parentRect = Rect(0.0f, 0.0f, windowSize.x, windowSize.y);
	}
	else
	{
		RectTransform* parent = static_cast<RectTransform*>(mParent);
		parentRect = parent->GetRect();
	}
	return RectUtil::CalcLocalRect(parentRect, mAnchorMin, mAnchorMax, mSizeDelta, mPivot);
}

Rect RectTransform::GetWorldRect(bool includeChildren)
{
	Rect localRect = GetRect();
	
	// "min" and "max" here should correlate to the min and max of the quad used for rendering.
	// Ex: A quad centered on origin should use min of -(mSize / 2) and max of (mSize / 2).
	// Ex: A quad with bottom-left corner at origin should use (0, 0, 0) and mSize respectively.
	// Our UI quad has a min of (0, 0) and a max of (1, 1).
	Vector2 min = localRect.GetMin();
	Vector2 max = localRect.GetMax();
	
	// Transform those points based on this transform's parents and scale/rotation/translation.
	min = LocalToWorldPoint(min);
	max = LocalToWorldPoint(max);
    Rect worldRect(min, max);
    
    // Let's say you want to get the world rect for a RectTransform, but it has a bunch of children that make up its contents.
    // This allows you to get a rect that contains all the children as well.
    if(includeChildren)
    {
        for(auto& child : mChildren)
        {
            if(child->IsTypeOf(RectTransform::GetType()) && child->IsActiveAndEnabled())
            {
                RectTransform* childRT = static_cast<RectTransform*>(child);
                worldRect.Contain(childRT->GetWorldRect(true));
            }
        }
    }
	
	// Return the rect.
    return worldRect;
}

void RectTransform::MoveInsideRect(const Rect& other)
{
    // Calculate our rect, taking into account children.
    Rect ourRect = GetWorldRect(true);
    
    Vector2 min = ourRect.GetMin();
    Vector2 max = ourRect.GetMax();
    
    Vector2 otherMin = other.GetMin();
    Vector2 otherMax = other.GetMax();

    // If our rect is outside of other rect, apply a diff to move it back inside.
    Vector2 anchoredPos = GetAnchoredPosition();
    if(min.x < otherMin.x)
    {
        anchoredPos.x += (otherMin.x - min.x);
    }
    if(max.x > otherMax.x)
    {
        anchoredPos.x += (otherMax.x - max.x);
    }
    if(min.y < otherMin.y)
    {
        anchoredPos.y += (otherMin.y - min.y);
    }
    if(max.y > otherMax.y)
    {
        anchoredPos.y += (otherMax.y - max.y);
    }
    SetAnchoredPosition(anchoredPos);

    // Can be helpful to visualize how this works.
    //Debug::DrawScreenRect(ourRect, Color32::Green);
    //Debug::DrawScreenRect(other, Color32::Red);
}

void RectTransform::CalcLocalPosition()
{
	Rect parentRect;
	Vector2 parentPivot;
	if(mParent == nullptr || !mParent->IsTypeOf(RectTransform::GetType()))
	{
        Vector2 windowSize = Window::GetSize();
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
		Rect screenRect = GetWorldRect();
        Debug::DrawScreenRect(screenRect, Color32::Cyan);

        // Also draw axes at pivot point.
        Vector2 pivotPos(screenRect.GetMin().x + screenRect.GetSize().x * mPivot.x, screenRect.GetMin().y + screenRect.GetSize().y * mPivot.y);
        Debug::DrawAxes(pivotPos);
	}
}
