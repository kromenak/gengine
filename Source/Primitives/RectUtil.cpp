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
	float leftDelta = sizeDelta.x * pivot.x;
	float rightDelta = sizeDelta.x * (1.0f - pivot.x);
	float bottomDelta = sizeDelta.y * pivot.y;
	float topDelta = sizeDelta.y * (1.0f - pivot.y);
	
	// Calculate final rect width/height.
	float rectWidth = anchorRect.GetSize().x + leftDelta + rightDelta;
	float rectHeight = anchorRect.GetSize().y + bottomDelta + topDelta;
	
	//Vector2 anchorPos = anchorRect.GetMin() + (anchorRect.GetPoint(pivot)) + anchoredPosition;
	//float rectXPos = anchorPos.x - (rectWidth * pivot.x);
	//float rectYPos = anchorPos.y - (rectHeight * pivot.y);
	
	// Local x/y pos of the rect is dictated by pivot as well.
	// If pivot is (0,0), lower-left corner or rect is (0,0). If pivot is (1,1), lower-left corner is (-width, -height).
	float rectXPos = -(rectWidth * pivot.x);
	float rectYPos = -(rectHeight * pivot.y);

    // Especially when dealing with UI and 2D text rendering, it can be important to maintain "pixel perfect" positioning of rects.
    // For the time being, I'm going to do this for ALL rects, but it could make sense to add a "pixelPerfect" parameter, or do this in the caller when needed.
    rectXPos = Math::Round(rectXPos);
    rectYPos = Math::Round(rectYPos);
    rectWidth = Math::Round(rectWidth);
    rectHeight = Math::Round(rectHeight);
	
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
