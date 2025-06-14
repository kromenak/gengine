//
// Clark Kromenaker
//
// A triangle is three non-collinear points.
//
#pragma once
#include "Vector3.h"

class Triangle
{
public:
    Triangle() = default;
    Triangle(const Vector3& p0, const Vector3& p1, const Vector3& p2);

    Vector3& operator[](int i) { return (&p0)[i]; }
    const Vector3& operator[](int i) const { return (&p0)[i]; }

    Vector3 GetNormal(bool clockwise = true, bool leftHanded = true) const;
    Vector3 GetCenter() const;

    bool ContainsPoint(const Vector3& point) const;
    Vector3 GetClosestPoint(const Vector3& point) const;

    // Triangle tests may need to be done on hundreds or thousands of triangles per frame.
    // It may be desirable to utilize triangle algorithms without creating a triangle instance.
    static Vector3 GetNormal(const Vector3& p0, const Vector3& p1, const Vector3& p2, bool clockwise = true, bool leftHanded = true);
    static bool ContainsPoint(const Vector3& p0, const Vector3& p1, const Vector3& p2, const Vector3& point);
    static Vector3 GetClosestPoint(const Vector3& p0, const Vector3& p1, const Vector3& p2, const Vector3& point);

    // A triangle consists of three points.
    // This class doesn't guarantee these points are valid/collinear (in fact, the default constructor gives an invalid triangle!).
    // These points are assumed to be in clockwise order:
    // p0
    // | \
    // |  \
    // |   \
    // p2---p1
    Vector3 p0;
    Vector3 p1;
    Vector3 p2;
};
