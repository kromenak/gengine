//
// Clark Kromenaker
//
// Represents a mathematical primitive "ray". A ray has a starting
// position, a direction, and extends infinitely in that direction.
//
#pragma once
#include "Vector3.h"

class Ray
{
public:
    Ray() = default;
    Ray(const Vector3& origin, const Vector3& direction);

    Vector3 GetPoint(float t) const { return origin + direction * t; }

    bool ContainsPoint(const Vector3& point) const;
    Vector3 GetClosestPoint(const Vector3& point) const;

    // Ray is defined as an origin and a direction.
    Vector3 origin;
    Vector3 direction;
};
