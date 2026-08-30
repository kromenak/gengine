#include "RectUtil.h"

Rect RectUtil::CalcAnchorRect(const Rect& parentRect, const Vector2& anchorMin, const Vector2& anchorMax)
{
    // Get points correlating to min/max anchors in parent rect.
    // Then, construct a rect from those two points!
    return Rect(parentRect.GetPoint(anchorMin), parentRect.GetPoint(anchorMax));
}

Rect RectUtil::CalcLocalRect(const Rect& parentRect, const Vector2& anchorMin, const Vector2& anchorMax,
                             const Vector2& sizeDelta, const Vector2& pivot, bool pixelPerfect)
{
    // Calculate the anchor rect, which is some rect relative to parent rect based on min/max anchors.
    Rect anchorRect = RectUtil::CalcAnchorRect(parentRect, anchorMin, anchorMax);

    // Calculate final rect width/height, which is just the anchor rect size plus the size delta.
    float rectWidth = anchorRect.GetSize().x + sizeDelta.x;
    float rectHeight = anchorRect.GetSize().y + sizeDelta.y;

    // Local x/y pos of the rect is dictated by pivot as well.
    // If pivot is (0,0), lower-left corner of rect is (0,0). If pivot is (1,1), lower-left corner is (-width, -height).
    float rectXPos = -(rectWidth * pivot.x);
    float rectYPos = -(rectHeight * pivot.y);

    // Especially when dealing with UI and 2D text rendering, it can be important to maintain "pixel perfect" positioning of rects.
    // For the time being, I'm going to do this for ALL rects, but it could make sense to add a "pixelPerfect" parameter, or do this in the caller when needed.
    if(pixelPerfect)
    {
        rectXPos = Math::Round(rectXPos);
        rectYPos = Math::Round(rectYPos);
        rectWidth = Math::Round(rectWidth);
        rectHeight = Math::Round(rectHeight);
    }

    // Construct the final thing - a rect relative to pivot point in local space!
    return Rect(rectXPos, rectYPos, rectWidth, rectHeight);
}

Vector2 RectUtil::CalcLocalPosition(const Rect& parentRect, const Vector2& parentPivot, const Vector2& anchorMin, const Vector2& anchorMax,
                                    const Vector2& anchoredPosition, const Vector2& pivot, bool pixelPerfect)
{
    Vector2 parentOriginPos = parentRect.GetPoint(parentPivot);

    // Calculate the anchor rect, which is some rect relative to parent rect based on min/max anchors.
    Rect anchorRect = RectUtil::CalcAnchorRect(parentRect, anchorMin, anchorMax);
    Vector2 anchorPos = anchorRect.GetPoint(pivot) + anchoredPosition;

    Vector2 localPosition = anchorPos - parentOriginPos;
    if(pixelPerfect)
    {
        localPosition.x = Math::Round(localPosition.x);
        localPosition.y = Math::Round(localPosition.y);
    }
    return localPosition;
}
