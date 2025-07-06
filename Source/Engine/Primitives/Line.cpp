#include "Line.h"

/*static*/ Line Line::FromPointAndDir(const Vector3& p0, const Vector3& dir)
{
    // This can't be a constructor variant b/c the argument types are the same :P
    return Line(p0, p0 + dir);
}

Line::Line(const Vector3& p0, const Vector3& p1) :
    p0(p0),
    p1(p1)
{

}

bool Line::ContainsPoint(const Vector3& point) const
{
    // Convert line representation to slope-intercept form (y = mx + b).
    float m = (p1.y - p0.y) / (p1.x - p0.x);
    float b = p0.y - m * p0.x;

    // If point is on this line, mx + b - y == 0.
    return Math::IsZero(m * point.x + b - point.y);
}

Vector3 Line::GetClosestPoint(const Vector3& point) const
{
    // See LineSegment for explanation of this algorithm!
    Vector3 startToEnd = p1 - p0;
    Vector3 startToPoint = point - p0;
    return GetPoint(Vector3::Dot(startToPoint, startToEnd) / Vector3::Dot(startToEnd, startToEnd));
}