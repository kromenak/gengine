//
// Clark Kromenaker
//
// A line extends infinitely in two directions.
//
// Because a line is infinite, the definition of "t" is somewhat unclear:
//  - t = 0 is always the start point (p0)
//  - Positive t extends in direction from p0 to p1
//  - t is a percentage of the distance between p0 and p1
// In a lot of cases, it probably makes sense to have p1-p0 be a unit vector, but sometimes not!
//
#pragma once
#include "Vector3.h"

class Line
{
public:
    static Line FromPointAndDir(const Vector3& p0, const Vector3& dir);

    Line() = default;
    Line(const Vector3& p0, const Vector3& p1);

    Vector3 GetPoint(float t) const { return p0 + ((p1 - p0) * t); }

    bool ContainsPoint(const Vector3& point) const;
    Vector3 GetClosestPoint(const Vector3& point) const;

    // Line is defined by any two points.
    Vector3 p0;
    Vector3 p1;
};