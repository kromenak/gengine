//
// Clark Kromenaker
//
// A sphere in 3D space.
//
#pragma once
#include "Vector3.h"

class Sphere
{
public:
    Sphere() = default;
    Sphere(const Vector3& center, float radius);

    bool ContainsPoint(const Vector3& point) const;
    Vector3 GetClosestSurfacePoint(const Vector3& point) const;

    // Sphere is defined as just a center point and radius.
    Vector3 center;
    float radius = 0.0f;
};
