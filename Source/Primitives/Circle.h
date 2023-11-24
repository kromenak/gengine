//
// Clark Kromenaker
//
// A circle - very similar to a sphere, but one less dimension!
//
#pragma once
#include "Vector2.h"

class Circle
{
public:
    Circle() = default;
    Circle(const Vector2& center, float radius);
	
	bool ContainsPoint(const Vector2& point) const;
	Vector2 GetClosestSurfacePoint(const Vector2& point) const;
	
	// Defined as a center point and radius.
	Vector2 center;
	float radius = 0.0f;
};
