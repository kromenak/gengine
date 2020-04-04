//
// Sphere.cpp
//
// Clark Kromenaker
//
#include "Sphere.h"

Sphere::Sphere(const Vector3& center, float radius) :
	mCenter(center),
	mRadius(radius)
{
	mRadius = Math::Max(0.0f, mRadius);
}

bool Sphere::ContainsPoint(const Vector3& point) const
{
	// I *think* a point ON the surface of the sphere should be considered contained in the sphere...
	return (point - mCenter).GetLengthSq() <= mRadius * mRadius;
}

Vector3 Sphere::GetClosestSurfacePoint(const Vector3& point) const
{
	return mCenter + ((point - mCenter).Normalize() * mRadius);
}
