#include "Circle.h"

Circle::Circle(const Vector2& center, float radius) :
	center(center),
	radius(radius)
{
	// Radius must not be negative.
	this->radius = Math::Max(0.0f, radius);
}

bool Circle::ContainsPoint(const Vector2& point) const
{
	// I *think* a point ON the surface of the sphere should be considered contained in the sphere...
	return (point - center).GetLengthSq() <= radius * radius;
}

Vector2 Circle::GetClosestSurfacePoint(const Vector2& point) const
{
	return center + ((point - center).Normalize() * radius);
}
