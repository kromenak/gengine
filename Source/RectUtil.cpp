//
// RectUtil.cpp
//
// Clark Kromenaker
//
#include "RectUtil.h"

Rect RectUtil::CalcAnchorRect(Rect parentRect, Vector2 anchorMin, Vector2 anchorMax)
{
	// Get points correlating to min/max anchors in parent rect.
	// Then, construct a rect from those two points!
	return Rect(parentRect.GetPoint(anchorMin), parentRect.GetPoint(anchorMax));
}

Rect RectUtil::CalcLocalRect(Rect parentRect, Vector2 anchorMin, Vector2 anchorMax, Vector2 sizeDelta, Vector2 pivot)
{
	// Calculate the anchor rect, which is some rect relative to parent rect based on min/max anchors.
	Rect anchorRect = RectUtil::CalcAnchorRect(parentRect, anchorMin, anchorMax);
	
	// SizeDelta defines offsets on left/right/bottom/top sides from the edges of the anchor rect calculated above.
	// Pivot also affects how much of sizeDelta affects left vs. right sides.
	// Ex: if sizeDelta.x == 100, and pivot is 0.5f, it means left/right sides each get 50. If pivot is 0.8, left side gets 80, right side gets 20.
	float leftDelta = sizeDelta.GetX() * pivot.GetX();
	float rightDelta = sizeDelta.GetX() * (1.0f - pivot.GetX());
	float bottomDelta = sizeDelta.GetY() * pivot.GetY();
	float topDelta = sizeDelta.GetY() * (1.0f - pivot.GetY());
	
	// Calculate final rect width/height.
	float rectWidth = anchorRect.GetSize().GetX() + leftDelta + rightDelta;
	float rectHeight = anchorRect.GetSize().GetY() + bottomDelta + topDelta;
	
	//Vector2 anchorPos = anchorRect.GetMin() + (anchorRect.GetPoint(pivot)) + anchoredPosition;
	//float rectXPos = anchorPos.GetX() - (rectWidth * pivot.GetX());
	//float rectYPos = anchorPos.GetY() - (rectHeight * pivot.GetY());
	
	// Local x/y pos of the rect is dictated by pivot as well.
	// If pivot is (0,0), lower-left corner or rect is (0,0). If pivot is (1,1), lower-left corner is (-width, -height).
	float rectXPos = -(rectWidth * pivot.GetX());
	float rectYPos = -(rectHeight * pivot.GetY());
	
	// Construct the final thing - a rect relative to pivot point in local space!
	return Rect(rectXPos, rectYPos, rectWidth, rectHeight);
}

Vector3 RectUtil::CalcLocalPosition(Rect parentRect, Vector2 parentPivot,
		  Vector2 anchorMin, Vector2 anchorMax, Vector2 anchoredPosition, Vector2 pivot)
{
	Vector2 parentOriginPos = parentRect.GetPoint(parentPivot);
	
	// Calculate the anchor rect, which is some rect relative to parent rect based on min/max anchors.
	Rect anchorRect = RectUtil::CalcAnchorRect(parentRect, anchorMin, anchorMax);
	Vector2 anchorPos = anchorRect.GetPoint(pivot) + anchoredPosition;
	
	Vector2 localPosition = anchorPos - parentOriginPos;
	return localPosition;
}
