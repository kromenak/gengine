//
// Ray.cpp
//
// Clark Kromenaker
//
#include "Ray.h"

Ray::Ray(const Vector3& origin, const Vector3& direction) :
    mOrigin(origin),
    mDirection(direction)
{
    
}

bool Ray::IntersectsTriangle(const Vector3& p0, const Vector3& p1, const Vector3& p2, Vector3& outHitPos) const
{
	// Calculate two vectors from p0 to p1/p2.
	Vector3 e1 = p1 - p0;
	Vector3 e2 = p2 - p0;
	
	Vector3 p = Vector3::Cross(mDirection, e2);
	float a = Vector3::Dot(e1, p);
	
	// If zero, means ray is parallel to triangle plane, which is not an intersection.
	if(Math::IsZero(a)) { return false; }
	
	float f = 1.0f / a;
	
	Vector3 s = mOrigin - p0;
	float u = f * Vector3::Dot(s, p);
	if(u < 0.0f || u > 1.0f) { return false; }
	
	Vector3 q = Vector3::Cross(s, e1);
	float v = f * Vector3::Dot(mDirection, q);
	if(v < 0.0f || u + v > 1.0f) { return false; }
	
	float t = f * Vector3::Dot(e2, q);
	if(t < 0) { return false; }
	
	// We DID intersect the triangle. Return the point of intersection.
	outHitPos = GetPosition(t);
	return true;
}
