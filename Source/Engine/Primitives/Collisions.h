//
// Clark Kromenaker
//
// A static class to easily access collision checks between a variety of primitives.
//
#pragma once
#include <cfloat>
#include <string>

class Actor;
class AABB;
class Frustum;
class Line;
class LineSegment;
class Plane;
class Ray;
class Rect;
class Sphere;
class Triangle;
class Vector2;
class Vector3;

struct RaycastHit
{
    // The "t" value at which the hit occurred.
    float t = FLT_MAX;

    // A name/identifier for the thing hit.
    std::string name;

    // An actor hit.
    Actor* actor = nullptr;
};

//
// Intersection tests between various primitives.
// These are all "instantaneous" point-in-time checks between the two primitives.
//
// All functions return true/false (did they interect or not?).
// Most functions also have out params for retrieving: point of intersection, "t values" to calculate point of intersection, offset to stop intersection, etc.
//
namespace Intersect
{
    // Sphere
    bool TestSphereSphere(const Sphere& s1, const Sphere& s2);
    bool TestSphereAABB(const Sphere& s, const AABB& aabb);
    bool TestSpherePlane(const Sphere& s, const Plane& p);
    bool TestSphereTriangle(const Sphere& s, const Triangle& t, Vector3& intersection);
    bool TestSphereLine(const Sphere& s, const Line& l, int& numIntersect, float& tIntersect1, float& tIntersect2);

    // AABB
    bool TestAABBAABB(const AABB& aabb1, const AABB& aabb2);
    //TestAABBPlane

    // Plane
    bool TestPlanePlane(const Plane& p1, const Plane& p2);
    bool TestPlaneLineSegment(const Plane& p, const LineSegment& ls, float& outLineSegmentT);

    // Ray
    bool TestRayAABB(const Ray& r, const AABB& aabb, float& outRayT);
    bool TestRayTriangle(const Ray& r, const Triangle& t, float& outRayT);
    bool TestRayTriangle(const Ray& r, const Vector3& p0, const Vector3& p1, const Vector3& p2, float& outRayT);
    bool TestRayTriangle(const Ray& r, const Vector3& p0, const Vector3& p1, const Vector3& p2, float& outRayT, float& outU, float& outV);

    //bool TestRaySphere(const Ray& r, const Sphere& s);
    //bool TestRayPlane(const Ray& r, const Plane& p);

    // LineSegment
    //bool TestLineSegmentSphere(const LineSegment& ls, const Sphere& s);
    //bool TestLineSegmentAABB(const LineSegment& ls, const AABB& aabb);
    //bool TestLineSegmentPlane(const LineSegment& ls, const Plane& p);

    // Line
    bool LineLine2D(const Vector2& line0P0, const Vector2& line0P1, const Vector2& line1P0, const Vector2& line1P1, float& outLine0T);
    bool LineSegmentLineSegment2D(const Vector2& line0P0, const Vector2& line0P1, const Vector2& line1P0, const Vector2& line1P1, float& outLine0T);
    bool LineSegmentRect2D(const Rect& rect, const Vector2& lineP0, const Vector2& lineP1, float& outLineEnterT, float& outLineExitT);

    // Frustum
    bool TestFrustumLineSegment(const Frustum& f, const LineSegment& ls);
}

//
// Collision tests are focused on continuous checks against one or more moving objects.
//
// Collision functions are also a bit different because they'll usually ignore pre-existing intersections and just focus on future intersections.
// If you need to know if something is intersecting RIGHT NOW, use Intersect functions.
// If you want to stop a moving object from going through something, use Collision functions.
//
namespace Collide
{
    bool SphereTriangle(const Sphere& sphere, const Triangle& triangle, const Vector3& sphereVelocity, float& outSphereT, Vector3& outCollisionNormal);
}