//
// Plane.cpp
//
// Clark Kromenaker
//
#include "Plane.h"

Plane::Plane(const Vector3& normal, float distance) :
    normal(normal),
    distance(distance)
{
	
}

Plane::Plane(float normalX, float normalY, float normalZ, float distance) :
    normal(normalX, normalY, normalZ),
    distance(distance)
{
    
}

Plane::Plane(const Vector3& p0, const Vector3& p1, const Vector3& p2)
{
	// Calculate normal using cross product.
    // NOTE: assuming a clockwise winding order for the points.
	normal = Vector3::Cross(p1 - p0, p2 - p0);
	normal.Normalize();
	
    // Plane equation is "n dot p + d = 0", therefore "n dot p = -d" or "-(n dot p) = d".
    // Any of the three points works here.
	distance = -Vector3::Dot(normal, p0);
}

float Plane::GetSignedDistance(const Vector3& point) const
{
    // A point must be in front of, behind, or on the plane.
    // Signed distance (also sometimes called "the plane test") is the distance from the plane,
    // but it is positive if point is in front of the plane, negative if behind, and zero if on the plane.
    return Vector3::Dot(normal, point) + distance;
}

float Plane::GetDistance(const Vector3& point) const
{
    // Absolute value of signed distance does the trick.
    return Math::Abs(GetSignedDistance(point));
}

bool Plane::ContainsPoint(const Vector3& point) const
{
    return Math::IsZero(GetSignedDistance(point));
}

Vector3 Plane::GetClosestPoint(const Vector3& point) const
{
    // Get a vector from plane surface to the point.
    Vector3 planeToPoint = GetSignedDistance(point) * normal;
    
    // Negate to get vector from point to plane.
    // Add to point to get a point on the plane!
    return point - planeToPoint; // Equivalent to "point + (-planeToPoint)"
}

/*static*/ bool Plane::GetIntersectionPoint(const Plane& p1, const Plane& p2, const Plane& p3, Vector3& outPoint)
{
	// Taken from https://gdbooks.gitbooks.io/3dcollisions/content/Chapter1/three_plane_intersection.html
    Vector3 p1Normal = p1.normal;
	Vector3 p2Normal = p2.normal;
	Vector3 p3Normal = p3.normal;
	
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
	
	Vector3 d(p1.distance,
			  p2.distance,
			  p3.distance);
	Vector3 v = Vector3::Cross(m1, d);
	
	// Calculate point.
	outPoint = Vector3(Vector3::Dot(d, u) / denom,
				   Vector3::Dot(m3, v) / denom,
				   -Vector3::Dot(m2, v) / denom);
	return true;
}
