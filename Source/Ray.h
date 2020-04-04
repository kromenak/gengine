//
// Ray.h
//
// Clark Kromenaker
//
// Represents a mathematical primitive "ray". A ray has a starting
// position, a direction, and extends infinitely in that direction.
//
#pragma once
#include "Vector3.h"

class Ray
{
public:
    Ray() { }
    Ray(const Vector3& origin, const Vector3& direction);
    
    Vector3 GetPoint(float t) const { return mOrigin + mDirection * t; }
    
	bool ContainsPoint(const Vector3& point) const;
	Vector3 GetClosestPoint(const Vector3& point) const;
	
	bool IntersectsTriangle(const Vector3& p0, const Vector3& p1, const Vector3& p2, Vector3& outHitPos) const;
	
	Vector3 GetOrigin() const { return mOrigin; }
    Vector3 GetDirection() const { return mDirection; }
	
private:
	// Ray is defined as an origin and a direction.
    Vector3 mOrigin;
    Vector3 mDirection;
};
