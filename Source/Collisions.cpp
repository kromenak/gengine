//
// Collisions.cpp
//
// Clark Kromenaker
//
#include "Collisions.h"

#include "AABB.h"
#include "Plane.h"
#include "Sphere.h"
#include "Triangle.h"

/*static*/ bool Collisions::TestSphereSphere(const Sphere& s1, const Sphere& s2)
{
	// Get squared distance between centers of spheres.
	float centersDistSq = (s1.GetCenter() - s2.GetCenter()).GetLengthSq();
	
	// Get squared length of radii summed.
	float radiiDistSq = s1.GetRadius() + s2.GetRadius();
	radiiDistSq *= radiiDistSq;
	
	// Spheres must be colliding if distance between centers is less than sum of radii.
	return centersDistSq <= radiiDistSq;
}

/*static*/ bool Collisions::TestSphereAABB(const Sphere& s, const AABB& aabb)
{
	// Get closest point on AABB to center of sphere.
	Vector3 closestAABBPoint = aabb.GetClosestPoint(s.GetCenter());
	
	// If squared distance from sphere center to closest point is less than squared radius, we have an intersection!
	float distSq = (s.GetCenter() - closestAABBPoint).GetLengthSq();
	return distSq < s.GetRadius() * s.GetRadius();
}

/*static*/ bool Collisions::TestSpherePlane(const Sphere& s, const Plane& p)
{
	// Get closest point on plane to the sphere.
	Vector3 closestPlanePoint = p.GetClosestPoint(s.GetCenter());
	
	// If squared distance from center to closest point is less than squared radius, we have an intersection!
	float distSq = (s.GetCenter() - closestPlanePoint).GetLengthSq();
	return distSq < s.GetRadius() * s.GetRadius();
}

/*static*/ bool Collisions::TestSphereTriangle(const Sphere& s, const Triangle& t)
{
	// Get closest point on triangle to sphere center.
	Vector3 pointOnTriangle = t.GetClosestPoint(s.GetCenter());
	
	// Get distance from center to point on triangle.
	float distSq = (pointOnTriangle - s.GetCenter()).GetLengthSq();
	return distSq < s.GetRadius() * s.GetRadius();
}

/*static*/ bool Collisions::TestAABBAABB(const AABB& aabb1, const AABB& aabb2)
{
	// There are 4 cases where the AABBs are not intersecting.
	bool case1 = aabb1.GetMax().x < aabb2.GetMin().x;
	bool case2 = aabb1.GetMin().x > aabb2.GetMax().x;
	bool case3 = aabb1.GetMax().y < aabb2.GetMin().y;
	bool case4 = aabb1.GetMin().y > aabb2.GetMax().y;
	
	// If none of those cases are true, they must be intersecting.
	return !case1 && !case2 && !case3 && !case4;
}

/*static*/ bool TestPlanePlane(const Plane& p1, const Plane& p2)
{
	// Planes don't intersect if they are parallel to one another.
	// If not parallel, the planes DO intersect.
	
	// If planes do intersect, the line of intersection is along this vector.
	// If parallel, normals are facing same direction or opposite - in both cases, cross result would be zero.
	Vector3 lineDir = Vector3::Cross(p1.GetNormal(), p2.GetNormal());
	
	// If not zero, there is an intersection.
	return !Math::IsZero(lineDir.GetLengthSq());
}
