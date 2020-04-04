//
// Sphere.h
//
// Clark Kromenaker
//
// A sphere in 3D space.
//
#pragma once
#include "Vector3.h"

class Sphere
{
public:
	Sphere() { }
	Sphere(const Vector3& center, float radius);
	
	bool ContainsPoint(const Vector3& point) const;
	Vector3 GetClosestSurfacePoint(const Vector3& point) const;
	
	Vector3 GetCenter() const { return mCenter; }
	float GetRadius() const { return mRadius; }
	
private:
	// Sphere is defined as just a center point and radius.
	Vector3 mCenter;
	float mRadius = 1.0f;
};
