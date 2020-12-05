//
// Collisions.cpp
//
// Clark Kromenaker
//
#include "Collisions.h"

#include "AABB.h"
#include "Plane.h"
#include "Ray.h"
#include "Sphere.h"
#include "Triangle.h"

/*static*/ bool Collisions::TestSphereSphere(const Sphere& s1, const Sphere& s2)
{
	// Get squared distance between centers of spheres.
	float centersDistSq = (s1.center - s2.center).GetLengthSq();
	
	// Get squared length of radii summed.
	float radiiDistSq = s1.radius + s2.radius;
	radiiDistSq *= radiiDistSq;
	
	// Spheres must be colliding if distance between centers is less than sum of radii.
	return centersDistSq <= radiiDistSq;
}

/*static*/ bool Collisions::TestSphereAABB(const Sphere& s, const AABB& aabb)
{
	// Get closest point on AABB to center of sphere.
	Vector3 closestAABBPoint = aabb.GetClosestPoint(s.center);
	
	// If squared distance from sphere center to closest point is less than squared radius, we have an intersection!
	float distSq = (s.center - closestAABBPoint).GetLengthSq();
	return distSq < s.radius * s.radius;
}

/*static*/ bool Collisions::TestSpherePlane(const Sphere& s, const Plane& p)
{
	// Get closest point on plane to the sphere.
	Vector3 closestPlanePoint = p.GetClosestPoint(s.center);
	
	// If squared distance from center to closest point is less than squared radius, we have an intersection!
	float distSq = (s.center - closestPlanePoint).GetLengthSq();
	return distSq < s.radius * s.radius;
}

/*static*/ bool Collisions::TestSphereTriangle(const Sphere& s, const Triangle& t, Vector3& intersection)
{
	// Get closest point on triangle to sphere center.
	Vector3 pointOnTriangle = t.GetClosestPoint(s.center);
	
	// Get distance from center to point on triangle.
	float distSq = (pointOnTriangle - s.center).GetLengthSq();
	bool intersects = distSq < s.radius * s.radius;
	if(intersects)
	{
		intersection = s.center - pointOnTriangle;
		intersection.Normalize();
		intersection *= s.radius - Math::Sqrt(distSq);
	}
	return intersects;
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

/*static*/ bool Collisions::TestPlanePlane(const Plane& p1, const Plane& p2)
{
	// Planes don't intersect if they are parallel to one another.
	// If not parallel, the planes DO intersect.
	
	// If planes do intersect, the line of intersection is along this vector.
	// If parallel, normals are facing same direction or opposite - in both cases, cross result would be zero.
	Vector3 lineDir = Vector3::Cross(p1.normal, p2.normal);
	
	// If not zero, there is an intersection.
	return !Math::IsZero(lineDir.GetLengthSq());
}

/*static*/ bool Collisions::TestRayAABB(const Ray& r, const AABB& aabb, RaycastHit& outHitInfo)
{
	Vector3 min = aabb.GetMin();
	Vector3 max = aabb.GetMax();
	Vector3 rayOrigin = r.origin;
	Vector3 rayDir = r.direction;
	
	float t1 = (min.x - rayOrigin.x) / rayDir.x;
    float t2 = (max.x - rayOrigin.x) / rayDir.x;
    float t3 = (min.y - rayOrigin.y) / rayDir.y;
    float t4 = (max.y - rayOrigin.y) / rayDir.y;
    float t5 = (min.z - rayOrigin.z) / rayDir.z;
    float t6 = (max.z - rayOrigin.z) / rayDir.z;

    float tMin = Math::Max(Math::Max(Math::Min(t1, t2), Math::Min(t3, t4)), Math::Min(t5, t6));
    float tMax = Math::Min(Math::Min(Math::Max(t1, t2), Math::Max(t3, t4)), Math::Max(t5, t6));

    // If tMax < 0, LINE is intersecting AABB, but RAY is not!
    if(tMax < 0)
	{
		return false;
    }

    // If tMin > tMax, ray doesn't intersect AABB.
    if(tMin > tMax)
	{
		return false;
    }

	// Pass "t" of intersection out to caller.
	outHitInfo.t = tMin >= 0.0f ? tMin : tMax;
	
	// Yep, they intersect.
	return true;
}

/*static*/ bool Collisions::TestRayTriangle(const Ray& r, const Triangle& t, RaycastHit& outHitInfo)
{
	return TestRayTriangle(r, t.p0, t.p1, t.p2, outHitInfo);
}

/*static*/ bool Collisions::TestRayTriangle(const Ray& r, const Vector3& p0, const Vector3& p1, const Vector3& p2, RaycastHit& outHitInfo)
{
	// Calculate two vectors from p0 to p1/p2.
	Vector3 e1 = p1 - p0;
	Vector3 e2 = p2 - p0;
	
	Vector3 p = Vector3::Cross(r.direction, e2);
	float a = Vector3::Dot(e1, p);
	
	// If zero, means ray is parallel to triangle plane, which is not an intersection.
	if(Math::IsZero(a)) { return false; }
	
	float f = 1.0f / a;
	
	Vector3 s = r.origin - p0;
	float u = f * Vector3::Dot(s, p);
	if(u < 0.0f || u > 1.0f) { return false; }
	
	Vector3 q = Vector3::Cross(s, e1);
	float v = f * Vector3::Dot(r.direction, q);
	if(v < 0.0f || u + v > 1.0f) { return false; }
	
	float t = f * Vector3::Dot(e2, q);
	if(t < 0) { return false; }
	
	// We DID intersect the triangle. Return the point of intersection.
	outHitInfo.t = t;
	return true;
}
