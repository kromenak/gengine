#include "Ray.h"

Ray::Ray(const Vector3& origin, const Vector3& direction) :
    origin(origin),
    direction(direction) // assumed that caller has normalized direction
{

}

bool Ray::ContainsPoint(const Vector3& point) const
{
    // Edge case: if origin is point...yep, ray contains this point!
    if(origin == point) { return true; }

    // Calculate normalized vector from origin to point.
    Vector3 pointDir = (point - origin).Normalize();

    // If dot product gives us 1, then these direction vectors point in the same direction!
    // So, that means the point is on the line!
    float dot = Vector3::Dot(direction, pointDir);
    return Math::AreEqual(dot, 1.0f);
}

Vector3 Ray::GetClosestPoint(const Vector3& point) const
{
    // Get a vector from start to point.
    Vector3 startToPoint = point - origin;

    // Project onto the line.
    Vector3 projection = Vector3::Dot(startToPoint, direction) * direction;

    // Determine "t" for projected position on the line segment.
    Vector3 end = origin + direction;
    float t = (projection - origin).GetLengthSq() / (end - origin).GetLengthSq();

    // This is where we differ from line segment algorithm: don't clamp, just greater than zero!
    t = Math::Max(0.0f, t);

    // Get the point that correlates to "t".
    return GetPoint(t);
}
