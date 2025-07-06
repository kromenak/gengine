//
// Clark Kromenaker
//
// A plane in 3D space. A plane extends infinitely, dividing 3D space into two halves.
//
// A plane is defined with the equation "n dot p + d = 0". This means:
// 1) A plane can be represented by its normal n and its distance d.
// 2) Entering a point p calculates if the point is on the plane, in front of the plane, or behind the plane.
//
// Another way to think of a plane is the surface defined by three non-colinear points (a triangle).
//
// This code assumes that the normal is, well, normalized. When this is true,
// distance equates to the distance of the plane from the origin.
//
// The distance can be signed, and the effect is not exactly intuitive.
// When distance is zero, the plane is at the origin of the coordinate system.
// As distance increases, the plane moves opposite the direction of the normal (resulting in normal facing origin for positive values).
// As distance decreases, the plane moves in direction of the normal (resulting in normal facing away from origin for negative values).
//
#pragma once
#include "Vector3.h"

class Vector4;
class Matrix4;

class Plane
{
public:
    Plane() = default;
    Plane(const Vector3& normal, float distance);
    Plane(const Vector3& normal, const Vector3& p0);
    Plane(float normalX, float normalY, float normalZ, float distance);
    Plane(const Vector4& normalAndDist);
    Plane(const Vector3& p0, const Vector3& p1, const Vector3& p2);

    void Normalize();
    static Plane Normalize(const Plane& plane);

    void Transform(const Matrix4& aToBTransform, bool preInverted = false);

    float GetSignedDistance(const Vector3& point) const;
    float GetDistance(const Vector3& point) const;

    bool ContainsPoint(const Vector3& point) const;
    Vector3 GetClosestPoint(const Vector3& point) const;

    // Given three planes, calculate point of intersection.
    static bool GetIntersectionPoint(const Plane& p1, const Plane& p2, const Plane& p3, Vector3& outPoint);

    // The plane's normal - assumed to be normalized.
    // The normal indicates the direction the plane is facing - the front side of the plane.
    // Points on the front side of the plane have positive signed distance, points on the back side have negative signed distance.
    Vector3 normal = Vector3::UnitX;

    // Distance of the plane. Note that negative distances are valid!
    // A positive distance means the plane is facing the origin, a negative distance means the plane is not facing the origin.
    float distance = 0.0f;
};
