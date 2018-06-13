//
// Plane.h
//
// Clark Kromenaker
//
// Represents a plane in 3D space. A plane extends infinitely in 2 directions.
//
// A plane can be represented by a normal and offset from origin.
// Another way to think of it is the surface defined by 3 points.
//
#pragma once
#include "Vector3.h"

class Plane
{
public:
    Plane();
    Plane(float normalX, float normalY, float normalZ, float distance);
    //Plane(const Vector3& p0, const Vector3& p1, const Vector3& p2);
    ~Plane() { }
    
    //TODO: Copy constructor and assignment
    //TODO: Comparison (== and !=)
    
    Vector3 FindClosestPointOnPlane(const Vector3& point) const;
    
    float Test(const Vector3& point) const
    {
        return Vector3::Dot(mNormal, point) + mDistance;
    }
    
    //TODO: Transform plane to another space.
    
    void Set(float x, float y, float z, float d);
    void Set(const Vector3& p0, const Vector3& p1, const Vector3& p2);
    
    const Vector3& GetNormal() const { return mNormal; }
    float GetDistanceFromOrigin() const { return mDistance; }
    
private:
    // We define a plane as normal and distance-from-origin values.
    // This is based off of plane equation: P dot N + d = 0
    Vector3 mNormal;
    float mDistance;
};
