//
// Plane.cpp
//
// Clark Kromenaker
//
#include "Plane.h"

Plane::Plane(const Vector3& normal, float distance)
{
	Set(normal.x, normal.y, normal.z, distance);
}

Plane::Plane(float normalX, float normalY, float normalZ, float distance)
{
    Set(normalX, normalY, normalZ, distance);
}

Plane::Plane(const Vector3& p0, const Vector3& p1, const Vector3& p2)
{
	// Calculate normal using cross product.
	mNormal = Vector3::Cross(p1 - p0, p2 - p0);
	mNormal.Normalize();
	
	// Distance from origin can be calculated using scalar projection. Any of p0/p1/p2 could be used here.
	mDistance = Vector3::Dot(mNormal, p0);
	
	// Negative distance implies that the normal is facing the wrong direction.
	// So, negate normal to flip it, and make distance positive.
	if(mDistance < 0.0f)
	{
		mNormal = -mNormal;
		mDistance = -mDistance;
	}
}

bool Plane::ContainsPoint(const Vector3& point) const
{
	return Math::IsZero(GetDistanceFromPlane(point));
}

Vector3 Plane::GetClosestPoint(const Vector3& point) const
{
	// (distToPlane * normal) gives us a vector from the plane surface to the point.
	// Negating that gives us a vector from the point to the plane's surface.
	Vector3 pointToPlane = -(GetDistanceFromPlane(point) * mNormal);
	
	// Adding pointToPlane moves us from point to the surface of the plane.
	return point + pointToPlane;
}

float Plane::GetDistanceFromPlane(const Vector3& point) const
{
	// We can use scalar projection (dot product) to get "distance from origin" along normal's direction.
	// But then, we must subtract plane's distance from origin to get "distance from plane", rather than distance from origin.
	return Vector3::Dot(mNormal, point) - mDistance;
}

/*static*/ bool Plane::GetIntersectionPoint(const Plane& p1, const Plane& p2, const Plane& p3, Vector3& outPoint)
{
	// Taken from https://gdbooks.gitbooks.io/3dcollisions/content/Chapter1/three_plane_intersection.html
	Vector3 p1Normal = p1.GetNormal();
	Vector3 p2Normal = p2.GetNormal();
	Vector3 p3Normal = p3.GetNormal();
	
	Vector3 m1(p1Normal.x, p2Normal.x, p3Normal.x);
    Vector3 m2(p1Normal.y, p2Normal.y, p3Normal.y);
    Vector3 m3(p1Normal.z, p2Normal.z, p3Normal.z);
	
    Vector3 u = Vector3::Cross(m2, m3);
    
	// Calculate denominator.
	// If zero, planes don't actually intersect.
    float denom = Vector3::Dot(m1, u);
    if(Math::IsZero(denom))
	{
		// Return false; don't touch outPoint.
		return false;
    }
	
	Vector3 d(p1.GetDistanceFromOrigin(),
			  p2.GetDistanceFromOrigin(),
			  p3.GetDistanceFromOrigin());
	Vector3 v = Vector3::Cross(m1, d);
	
	// Calculate point.
	outPoint = Vector3(Vector3::Dot(d, u) / denom,
				   Vector3::Dot(m3, v) / denom,
				   -Vector3::Dot(m2, v) / denom);
	return true;
}

void Plane::Set(float x, float y, float z, float d)
{
    float lengthSq = x * x + y * y + z * z;
    if(Math::IsZero(lengthSq))
    {
        mNormal = Vector3::UnitX;
        mDistance = 0.0f;
    }
    else
    {
        float recip = Math::InvSqrt(lengthSq);
        mNormal.x = x * recip;
        mNormal.y = y * recip;
        mNormal.z = z * recip;
        mDistance = d * recip;
    }
}
