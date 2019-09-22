//
// RectUtil.h
//
// Clark Kromenaker
//
// Utilities for dealing with Rects (and RectTransforms).
//
#pragma once
#include "Rect.h"
#include "Vector2.h"
#include "Vector3.h"

namespace RectUtil
{
	// Given parent rect and child anchors, calculates child's anchor rect in parent's local space.
	Rect CalcAnchorRect(Rect parentRect, Vector2 anchorMin, Vector2 anchorMax);
	
	// Given parent rect and child anchors/size/pivot, calculates rect in child's local space.
	Rect CalcLocalRect(Rect parentRect, Vector2 anchorMin, Vector2 anchorMax, Vector2 sizeDelta, Vector2 pivot);
	
	// Given parent rect/pivot and child's anchors/anchorPos/pivot, calculates child's local position in parent's space.
	Vector3 CalcLocalPosition(Rect parentRect, Vector2 parentPivot, Vector2 anchorMin, Vector2 anchorMax, Vector2 anchoredPosition, Vector2 pivot);
}
