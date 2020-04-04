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
    //TODO: Make sure direction is normalized?
}

bool Ray::ContainsPoint(const Vector3& point) const
{
	// Edge case: if origin is point...yep, ray contains this point!
	if(mOrigin == point) { return true; }
	
	// Calculate normalized vector from origin to point.
	Vector3 pointDir = (point - mOrigin).Normalize();
	
	// If dot product gives us 1, then these direction vectors point in the same direction!
	// So, that means the point is on the line!
	float dot = Vector3::Dot(mDirection, pointDir);
	return Math::AreEqual(dot, 1.0f);
}

Vector3 Ray::GetClosestPoint(const Vector3& point) const
{
	// Get a vector from start to point.
	Vector3 startToPoint = point - mOrigin;
	
	// Project onto the line.
	Vector3 projection = Vector3::Dot(startToPoint, mDirection) * mDirection;
	
	// Determine "t" for projected position on the line segment.
	Vector3 end = mOrigin + mDirection;
	float t = (projection - mOrigin).GetLengthSq() / (end - mOrigin).GetLengthSq();
	
	// This is where we differ from line segment algorithm: don't clamp, just greater than zero!
	t = Math::Max(0.0f, t);
	
	// Get the point that correlates to "t".
	return GetPoint(t);
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
	outHitPos = GetPoint(t);
	return true;
}
