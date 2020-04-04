//
// LineSegment.cpp
//
// Clark Kromenaker
//
#include "LineSegment.h"

LineSegment::LineSegment(const Vector3& start, const Vector3& end) :
	mStart(start),
	mEnd(end)
{
	
}

bool LineSegment::ContainsPoint(const Vector3& point) const
{
	// First, see if the point is on the line (not line segment).
	// Convert line representation to slope-intercept form (y = mx + b).
	float m = (mEnd.y - mStart.y) / (mEnd.x - mStart.x);
	float b = mStart.y - m * mStart.x;
	
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
	float t = (point - mStart).GetLengthSq() / (mEnd - mStart).GetLengthSq();
	return t >= 0.0f && t <= 1.0f;
}

Vector3 LineSegment::GetClosestPoint(const Vector3& point) const
{
	// Get a unit vector in the line direction.
	Vector3 dir = mEnd - mStart;
	dir.Normalize();
	
	// Get a vector from start to point.
	Vector3 startToPoint = point - mStart;
	
	// Project onto the line.
	Vector3 projection = Vector3::Dot(startToPoint, dir) * dir;
	
	// Determine "t" for projected position on the line segment.
	float t = (projection - mStart).GetLengthSq() / (mEnd - mStart).GetLengthSq();
	
	// If "t" was outside 0-1, then the closest point will be start/end.
	// So, we can just clamp t to valid bounds.
	t = Math::Clamp(t, 0.0f, 1.0f);
	
	// Get the point that correlates to "t".
	return GetPoint(t);
}
