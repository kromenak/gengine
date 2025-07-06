//
// Clark Kromenaker
//
// Utilities for dealing with Rects, primarily in relation to RectTransforms.
//
#pragma once
#include "Rect.h"
#include "Vector2.h"
#include "Vector3.h"

namespace RectUtil
{
    // Given parent rect and child anchors, calculates child's anchor rect in parent's local space.
    Rect CalcAnchorRect(const Rect& parentRect, const Vector2& anchorMin, const Vector2& anchorMax);

    // Given parent rect and child anchors/size/pivot, calculates rect in child's local space.
    Rect CalcLocalRect(const Rect& parentRect, const Vector2& anchorMin, const Vector2& anchorMax,
                       const Vector2& sizeDelta, const Vector2& pivot, bool pixelPerfect = true);

    // Given parent rect/pivot and child's anchors/anchorPos/pivot, calculates child's local position in parent's space.
    Vector3 CalcLocalPosition(const Rect& parentRect, const Vector2& parentPivot, const Vector2& anchorMin, const Vector2& anchorMax,
                              const Vector2& anchoredPosition, const Vector2& pivot, bool pixelPerfect = true);
}
