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
    
    Vector3 GetPosition(float t) { return mOrigin + mDirection * t; }
    
    Vector3 GetOrigin() { return mOrigin; }
    Vector3 GetDirection() { return mDirection; }
    
private:
    Vector3 mOrigin;
    Vector3 mDirection;
};
