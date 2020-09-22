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
	Plane() = default;
	Plane(const Vector3& normal, float distance);
    Plane(float normalX, float normalY, float normalZ, float distance);
    Plane(const Vector3& p0, const Vector3& p1, const Vector3& p2);
    
    Vector3 GetNormal() const { return mNormal; }
    float GetDistanceFromOrigin() const { return mDistance; }
    
	//TODO: Transform plane to another space.
    
	bool ContainsPoint(const Vector3& point) const;
	Vector3 GetClosestPoint(const Vector3& point) const;
	
	float GetDistanceFromPlane(const Vector3& point) const;
	
	// Given three planes, calculate point of intersection.
	static bool GetIntersectionPoint(const Plane& p1, const Plane& p2, const Plane& p3, Vector3& outPoint);
	
private:
    // We define a plane as normal and distance-from-origin values.
    // This is based off of plane equation: P dot N + d = 0
	Vector3 mNormal = Vector3::UnitX;
	float mDistance = 0.0f;
	
	void Set(float x, float y, float z, float d);
};
