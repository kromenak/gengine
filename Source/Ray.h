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
    
    //Ray(const Ray& other);
    //Ray operator=(const Ray& other);
    
    //bool operator==(const Ray& ray) const;
    //bool operator!=(const Ray& ray) const;
    
    Vector3 GetPosition(float t) const { return mOrigin + mDirection * t; }
    
    Vector3 GetOrigin() const { return mOrigin; }
    Vector3 GetDirection() const { return mDirection; }
	
	bool IntersectsTriangle(const Vector3& p0, const Vector3& p1, const Vector3& p2, Vector3& outHitPos) const;
	
private:
    Vector3 mOrigin;
    Vector3 mDirection;
};
