#include "Sphere.h"

Sphere::Sphere(const Vector3& center, float radius) :
    center(center),
    radius(radius)
{
    // Radius must not be negative.
    this->radius = Math::Max(0.0f, radius);
}

bool Sphere::ContainsPoint(const Vector3& point) const
{
    // I *think* a point ON the surface of the sphere should be considered contained in the sphere...
    return (point - center).GetLengthSq() <= radius * radius;
}

Vector3 Sphere::GetClosestSurfacePoint(const Vector3& point) const
{
    return center + ((point - center).Normalize() * radius);
}
