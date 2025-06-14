#include "Collisions.h"

#include "AABB.h"
#include "Debug.h"
#include "GMath.h"
#include "Line.h"
#include "Plane.h"
#include "Ray.h"
#include "Sphere.h"
#include "Triangle.h"
#include "Vector2.h"
#include "Vector3.h"

bool Intersect::TestSphereSphere(const Sphere& s1, const Sphere& s2)
{
    // Get squared distance between centers of spheres.
    float centersDistSq = (s1.center - s2.center).GetLengthSq();

    // Get squared length of radii summed.
    float radiiDistSq = s1.radius + s2.radius;
    radiiDistSq *= radiiDistSq;

    // Spheres must be colliding if distance between centers is less than sum of radii.
    return centersDistSq <= radiiDistSq;
}

bool Intersect::TestSphereAABB(const Sphere& s, const AABB& aabb)
{
    // Get closest point on AABB to center of sphere.
    Vector3 closestAABBPoint = aabb.GetClosestPoint(s.center);

    // If squared distance from sphere center to closest point is less than squared radius, we have an intersection!
    float distSq = (s.center - closestAABBPoint).GetLengthSq();
    return distSq < s.radius* s.radius;
}

bool Intersect::TestSpherePlane(const Sphere& s, const Plane& p)
{
    // Get closest point on plane to the sphere.
    Vector3 closestPlanePoint = p.GetClosestPoint(s.center);

    // If squared distance from center to closest point is less than squared radius, we have an intersection!
    float distSq = (s.center - closestPlanePoint).GetLengthSq();
    return distSq < s.radius* s.radius;
}

bool Intersect::TestSphereTriangle(const Sphere& s, const Triangle& t, Vector3& intersection)
{
    // Get closest point on triangle to sphere center.
    Vector3 pointOnTriangle = t.GetClosestPoint(s.center);

    // Get distance from center to point on triangle.
    float distSq = (pointOnTriangle - s.center).GetLengthSq();
    bool intersects = distSq < s.radius* s.radius;
    if(intersects)
    {
        intersection = s.center - pointOnTriangle;
        intersection.Normalize();
        intersection *= s.radius - Math::Sqrt(distSq);
    }
    return intersects;
}

bool Intersect::TestSphereLine(const Sphere& s, const Line& l, int& numIntersect, float& tIntersect1, float& tIntersect2)
{
    // Sphere intersects a line if distance to the nearest point on the line is less than the radius. Easy.

    // BUT if we need exact points of intersection, it's a bit tricker.
    // Find when sphere-to-line distance is exactly equal to the sphere radius.
    //   Radius = ||Point - SphereCenter||

    // Substitute in line equation:
    //   Radius = ||P0 - (P1 - P0)*t - SphereCenter||
    // Rearrange:
    //   Radius = ||P0 - SphereCenter - (P1 - P0)*t||
    // Substitute: X = P0-SphereCenter, Y = P1-P0
    //   Radius = ||X - Yt||
    // Rearrange:
    //   Radius^2 = ||X - Yt||^2
    //   Radius^2 = (X - Yt) * (X - Yt)         (b/c||v||^2 == v * v)
    //   Radius^2 = X*X + 2(X*Y)t + Y*Yt^2      (using FOIL)
    //   0 = Y*Yt^2 + 2(X*Y)t + X*X - Radius^2

    // What do you know - you can solve that with the quadratic forumula:
    // a = Y*Y, b = 2(X*Y), c = X*X - Radius^2
    // t = (-b +/- sqrt(b^2 - 4ac)) / 2a

    // Calculate our X and Y.
    Vector3 X = l.p0 - s.center;
    Vector3 Y = l.p1 - l.p0;

    // Use those to create a/b/c
    float a = Vector3::Dot(Y, Y);
    float b = 2 * Vector3::Dot(X, Y);
    float c = Vector3::Dot(X, X) - (s.radius * s.radius);

    // If descriminant is < 0, no intersection.
    float descriminant = (b * b) - (4 * a * c);
    if(descriminant < 0)
    {
        return false;
    }

    // Exacty zero is one point of intersection.
    // >0 is two points of intersection.
    if(descriminant == 0)
    {
        numIntersect = 1;
        tIntersect1 = -b / (2 * a);
    }
    else
    {
        numIntersect = 2;
        tIntersect1 = (-b + Math::Sqrt((b * b) - 4 * a * c)) / (2 * a);
        tIntersect2 = (-b - Math::Sqrt((b * b) - 4 * a * c)) / (2 * a);
    }
    return true;
}

bool Intersect::TestAABBAABB(const AABB& aabb1, const AABB& aabb2)
{
    // There are 4 cases where the AABBs are not intersecting.
    bool case1 = aabb1.GetMax().x < aabb2.GetMin().x;
    bool case2 = aabb1.GetMin().x > aabb2.GetMax().x;
    bool case3 = aabb1.GetMax().y < aabb2.GetMin().y;
    bool case4 = aabb1.GetMin().y > aabb2.GetMax().y;

    // If none of those cases are true, they must be intersecting.
    return !case1 && !case2 && !case3 && !case4;
}

bool Intersect::TestPlanePlane(const Plane& p1, const Plane& p2)
{
    // Planes don't intersect if they are parallel to one another.
    // If not parallel, the planes DO intersect.

    // If planes do intersect, the line of intersection is along this vector.
    // If parallel, normals are facing same direction or opposite - in both cases, cross result would be zero.
    Vector3 lineDir = Vector3::Cross(p1.normal, p2.normal);

    // If not zero, there is an intersection.
    return !Math::IsZero(lineDir.GetLengthSq());
}

bool Intersect::TestRayAABB(const Ray& r, const AABB& aabb, float& outRayT)
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
    outRayT = tMin >= 0.0f ? tMin : tMax;

    // Yep, they intersect.
    return true;
}

bool Intersect::TestRayTriangle(const Ray& r, const Triangle& t, float& outRayT)
{
    return TestRayTriangle(r, t.p0, t.p1, t.p2, outRayT);
}

bool Intersect::TestRayTriangle(const Ray& r, const Vector3& p0, const Vector3& p1, const Vector3& p2, float& outRayT)
{
    float u = 0.0f;
    float v = 0.0f;
    return TestRayTriangle(r, p0, p1, p2, outRayT, u, v);
}

bool Intersect::TestRayTriangle(const Ray& r, const Vector3& p0, const Vector3& p1, const Vector3& p2, float& outRayT, float& outU, float& outV)
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
    outU = f * Vector3::Dot(s, p);
    if(outU < 0.0f || outU > 1.0f) { return false; }

    Vector3 q = Vector3::Cross(s, e1);
    outV = f * Vector3::Dot(r.direction, q);
    if(outV < 0.0f || outU + outV > 1.0f) { return false; }

    float t = f * Vector3::Dot(e2, q);
    if(t < 0) { return false; }

    // We DID intersect the triangle. Return the point of intersection.
    outRayT = t;
    return true;
}

bool Intersect::LineLine2D(const Vector2& line0P0, const Vector2& line0P1, const Vector2& line1P0, const Vector2& line1P1, float& outLine0T)
{
    Vector2 d1 = line0P1 - line0P0;
    Vector2 d2 = line1P1 - line1P0;

    float denom = d2.y * d1.x - d2.x * d1.y;
    if(Math::IsZero(denom))
    {
        return false;
    }

    float dist = d2.x * (line0P0.y - line1P0.y) - d2.y * (line0P0.x - line1P0.x);
    dist /= denom;
    outLine0T = dist;
    return true;
}

bool Collide::SphereTriangle(const Sphere& sphere, const Triangle& triangle, const Vector3& sphereMoveOffset, float& outSphereT, Vector3& outCollisionNormal)
{
    // Adapted (after A LOT of head scratching and experimenting) from flipcode.com/archives/Moving_Sphere_VS_Triangle_Collision.shtml

    // Sphere must be moving towards triangle to collide. Even if actively intersecting, if not moving toward triangle front, no collision.
    // Allows moving in-bounds from out-of-bounds on a map, but not vice-versa, for example.
    Vector3 triNormal = triangle.GetNormal();
    if(Vector3::Dot(triNormal, sphereMoveOffset) >= 0)
    {
        return false;
    }

    // We'll do three passes to check collision with the triangle interior, the triangle vertices, and triangle edges.
    // As we go, we'll keep track of the "closest" collision we find a t-value for.
    bool collided = false;
    float smallestT = FLT_MAX;

    // PASS 1: SPHERE VS PLANE
    // Get plane containing triangle and see if sphere intersects that plane (and if so, if point of intersection is within triangle).
    // Observe also: if sphere DOES NOT intersect triangle plane, we early out b/c there's no chance of edge/vert collision in that case.
    {
        // Create plane from the triangle.
        Plane plane(triNormal, triangle.p0);

        // Get signed distance from plane to sphere center.
        // This value is positive if sphere is "in front of" plane (in direction normal is facing) and negative otherwise.
        float signedDistToPlane = plane.GetSignedDistance(sphere.center);

        // If sphere is behind plane and distance is farther than radius, sphere is not intersecting plane AND is on non-solid side of triangle anyway.
        // No possibility of intersecting with this triangle (not even an edge or vertex).
        if(signedDistToPlane < -sphere.radius)
        {
            return false;
        }

        // If sphere is intersecting plane, but is more on the negative side (signedDist < 0), we will skip the surface check.
        // But we still will need to check edge/vertex in this case.
        //if(signedDistToPlane >= sphere.radius)
        if(signedDistToPlane >= 0)
        {
            // The signed dist is sphere *center* to plane.
            // Subtract radius to get signed dist from sphere *surface* to plane.
            signedDistToPlane -= sphere.radius;

            // Use scalar projection to isolate amount of move offset in direction of normal (https://www.geogebra.org/m/XShfg9r8).
            // Since we want a positive distance here, negate normal so vectors are in same direction (remember, they are "roughly opposite" b/c we checked that earlier).6
            float offsetDistTowardsTri = Vector3::Dot(-triNormal, sphereMoveOffset);

            // t-value is then a %: dist to plane out of total dist we might try to move this frame.
            float t = signedDistToPlane / offsetDistTowardsTri;

            // If t > 1.0f, it means distance to plane is greater than the movement amount this frame.
            // In other words, the point of intersection with the plane is beyond our max movement - no collision!
            // So, we don't need to check anything else - it's not possible to intersect with the triangle in this case (even edges/verts).
            if(t > 1.0f)
            {
                return false;
            }

            // Use t-value to determine exact point on plane where collision occurs.
            Vector3 moveDir = Vector3::Normalize(sphereMoveOffset);
            Vector3 pointOnSphere = sphere.center + moveDir * sphere.radius;
            Vector3 pointOnPlane = pointOnSphere + sphereMoveOffset * t;

            // See if the point is inside the triangle. If so, this is a sphere intersection!
            if(triangle.ContainsPoint(pointOnPlane))
            {
                /*
                // Debug visualization for big negative t values. Trying to understand why that happens...
                if(t < -2.0f)
                {
                    Debug::DrawSphere(sphere, Color32::Green, 120.0f);
                    Debug::DrawLine(sphere.center, sphere.center + moveDir, Color32::Red, 120.0f);
                    Debug::DrawLine(sphere.center, pointOnPlane, Color32::Magenta, 120.0f);
                    Debug::DrawTriangle(triangle, Color32::Yellow, 120.0f);
                    Debug::DrawLine(triangle.GetCenter(), triangle.GetCenter() + triangle.GetNormal() * 5.0f, Color32::Yellow, 120.0f);
                }
                */

                collided = true;
                smallestT = t;
                outCollisionNormal = triNormal;

                // If needed: point on sphere where collision would occur, point on triangle where collision would occur.
                //Vector3 pointOnSphere = pointOnSphere;
                //Vector3 pointOnTriangle = pointOnPlane;
            }
        }
    }

    // PASS 2: SPHERE VS TRI VERTS
    // Even if PASS 1 yielded a collision, we should also check individual tri verts, since it might yield a BETTER/EARLIER collision.
    // Idea: create lines from triangle verts in movement direction, see if they intersect with the sphere.
    for(int i = 0; i < 3; ++i)
    {
        // To make a line, we need two point. One is obviously the triangle vertex!
        // The other will be based on the move direction of the sphere (going towards the sphere, so opposite of move dir).
        // This is hard to visualize, but think about how move direction affects the lines (or check out the Unity scene).
        Vector3 lineP0 = triangle[i];
        Vector3 lineP1 = lineP0 + (-sphereMoveOffset);
        Line line(lineP0, lineP1);

        // See if sphere intersects with this line.
        int numIntersect = 0;
        float intersectT1 = 0.0f;
        float intersectT2 = 0.0f;
        bool intersect = Intersect::TestSphereLine(sphere, line, numIntersect, intersectT1, intersectT2);
        if(!intersect)
        {
            continue;
        }

        // Could be one of two intersection points.
        // Use the smaller one (point of entry rather than point of exit).
        float t = intersectT1;
        if(numIntersect > 1 && intersectT2 < t)
        {
            t = intersectT2;
        }

        // See if this is a closer collision than any other found.
        if(t < smallestT && t >= 0.0f)
        {
            smallestT = t;
            collided = true;

            Vector3 pointOnSphere = line.GetPoint(t);
            outCollisionNormal = Vector3::Normalize(sphere.center - pointOnSphere);

            //Vector3 pointOnTriangle = triangle[i];
        }
    }

    // PASS 3: SPHERE VS TRI EDGES
    for(int i = 0; i < 3; ++i)
    {
        // Calculate next point's index, with wraparound.
        int j = i + 1;
        if(j == 3) { j = 0; }

        // Get two points of triangle to create an edge.
        Vector3 p0 = triangle[i];
        Vector3 p1 = triangle[j];

        // Create a third point opposite of move dir.
        Vector3 p2 = p1 + (-sphereMoveOffset);

        // Create a plane from these points.
        // Each plane is kind of like a side of a "tunnel" shaped like the triangle (though remember planes extend infinitely).
        Plane plane(p0, p1, p2);

        // If the sphere is not intersecting with the plane, we can ignore this edge.
        // We can check this by comparing the distance.
        float centerToPlaneDist = plane.GetDistance(sphere.center);
        if(centerToPlaneDist >= sphere.radius)
        {
            continue;
        }

        // Get closest point on edge plane to sphere center.
        Vector3 closestPointOnPlane = plane.GetClosestPoint(sphere.center);
        //Debug::DrawLine(sphere.center, closestPointOnPlane, Color32::Orange);

        // From that closest point on edge plane, now get the closest point on the edge line.
        // This point IS NOT necessarily on the triangle edge (remember, lines are infinite in both directions).
        Line edgeLine(p0, p1);
        Vector3 closestPointOnEdgeLine = edgeLine.GetClosestPoint(closestPointOnPlane);
        //Debug::DrawLine(closestPointOnPlane, closestPointOnEdgeLine, Color32::Green);

        // A right-triangle is formed, with the hypotenuse length equal to sphere radius, and another side's length equal to sphere-center-to-plane.
        // Using pythagorean theorem, we can calculate length of remaining side.
        float sideLength = Math::Sqrt((sphere.radius * sphere.radius) - (centerToPlaneDist * centerToPlaneDist));

        // The side length allows us to calculate the closest point on the sphere to the edge.
        Vector3 edgePlaneToEdgeLineDir = Vector3::Normalize(closestPointOnEdgeLine - closestPointOnPlane);
        Vector3 closestPointOnSphere = closestPointOnPlane + edgePlaneToEdgeLineDir * sideLength;
        //Debug::DrawLine(sphere.center, closestPointOnSphere, Color32::Magenta);

        // Determine whether we'll use x/y/z components for 2D line test.
        // We want to use the two SMALLEST components.
        // So, if normal is mostly in Z direction, we want to use X/Y.
        // If normal is mostly in X direction, use Y/Z.
        float absPlaneX = Math::Abs(plane.normal.x);
        float absPlaneY = Math::Abs(plane.normal.y);
        float absPlaneZ = Math::Abs(plane.normal.z);

        // Z is biggest, use X/Y.
        int a0 = 0, a1 = 1;
        if(absPlaneX > absPlaneY && absPlaneX > absPlaneZ)
        {
            // X is biggest, use Y/Z.
            a0 = 1;
            a1 = 2;
        }
        else if(absPlaneY > absPlaneZ)
        {
            // Y is biggest, use X/Z.
            a0 = 0;
            a1 = 2;
        }

        // Create a point near the closest point on the sphere, in the movement direction.
        // This allows us to create a line in that direction.
        Vector3 pointInMoveDir = closestPointOnSphere + sphereMoveOffset;

        // Do an intersection test between (a 2D line from sphere point in move dir) and (a 2D line along the edge of the triangle).
        float line0T = 0.0f;
        bool res = Intersect::LineLine2D(Vector2(closestPointOnSphere[a0], closestPointOnSphere[a1]), Vector2(pointInMoveDir[a0], pointInMoveDir[a1]),
                                           Vector2(p0[a0], p0[a1]), Vector2(p1[a0], p1[a1]), line0T);
        if(!res)
        {
            continue;
        }

        // Determine point of intersection along edge.
        Vector3 triEdgeIntersectPoint = closestPointOnSphere + sphereMoveOffset * line0T;

        // Make sure the intersect point is actually between the two edge points (and not outside the triangle).
        // We can detect this with dot product vects from intersection point to each tri point should face *away* from one another.
        if(Vector3::Dot(p0 - triEdgeIntersectPoint, p1 - triEdgeIntersectPoint) > 0)
        {
            continue;
        }

        // If this is a better result than either phase 1 or phase 2, use this!
        if(line0T < smallestT && line0T >= 0.0f)
        {
            collided = true;
            smallestT = line0T;
            outCollisionNormal = Vector3::Normalize(sphere.center - closestPointOnSphere);

            //Vector3 pointOnSphere = closestPointOnSphere;
            //Vector3 pointOnTriangle = triEdgeIntersectPoint;
        }
    }

    // Pass back smallest/nearest t-value.
    outSphereT = smallestT;

    // Return whether a collision occurred.
    return collided;
}