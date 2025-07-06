#include "LineSegment.h"

LineSegment::LineSegment(const Vector3& start, const Vector3& end) :
    start(start),
    end(end)
{

}

bool LineSegment::ContainsPoint(const Vector3& point) const
{
    // First, see if the point is on the line (not line segment).
    // Convert line representation to slope-intercept form (y = mx + b).
    float m = (end.y - start.y) / (end.x - start.x);
    float b = start.y - m * start.x;

    // If point is on this line, mx + b - y == 0.
    // If that's not true, we can early out.
    if(!Math::IsZero(m * point.x + b - point.y))
    {
        return false;
    }

    // OK, point is on the LINE, but is it on the LINE SEGMENT!?
    // We can calculate "t" in L(t) = Start + (End - Start) * t to figure this out.
    // Reorganize equation to t = (point - Start) / (End - Start).
    // If 0 <= t <= 1, then the point is on the line segment.
    float t = (point - start).GetLengthSq() / (end - start).GetLengthSq();
    return t >= 0.0f && t <= 1.0f;
}

Vector3 LineSegment::GetClosestPoint(const Vector3& point) const
{
    // Get vectors start-to-end and start-to-point.
    Vector3 startToEnd = end - start;
    Vector3 startToPoint = point - start;

    /*
    // This also works, and is maybe more straightforward logic, but is a bit more expensive.
    Vector3 dir = startToEnd.Normalize();
    Vector3 projection = Vector3::Dot(startToPoint, dir) * dir;
    return start + projection;
    */

    // Project "start to point" onto "start to end." That'll give us the closest point on the line.
    // Why does this work?
    // ((a dot b) / ||b||) is scalar projection (which projects length of startToPoint onto line)
    // (1 / ||b||) divides by length of b (with gives us fraction of line length - t value)
    // Combined, we get ((a dot b) / (||b|| * ||b||)). And ||b|| * ||b|| is equal to b dot b.
    // All together, we've got ((a dot b) / (b dot b)).
    float t = Vector3::Dot(startToPoint, startToEnd) / Vector3::Dot(startToEnd, startToEnd);
    t = Math::Clamp(t, 0.0f, 1.0f);
    return GetPoint(t);
}
