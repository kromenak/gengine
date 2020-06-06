//
// LineSegment.h
//
// Clark Kromenaker
//
// A line segment extends between two points (start and end points).
//
// Whereas a line extends infinitely in both directions, a line segment ends.
// Using the line equation L(t) = Start + (End - Start) * t, a line segment
// clamps t between 0-1, while a line does not.
//
#pragma once
#include "Vector3.h"

class LineSegment
{
public:
	LineSegment() { }
	LineSegment(const Vector3& start, const Vector3& end);
	
	Vector3 GetPoint(float t) const { return mStart + ((mEnd - mStart) * t); }
	
	bool ContainsPoint(const Vector3& point) const;
	Vector3 GetClosestPoint(const Vector3& point) const;
	
	Vector3 GetStart() const { return mStart; }
	Vector3 GetEnd() const { return mEnd; }
	
private:
	// Line segment is defined by start and end points.
	Vector3 mStart;
	Vector3 mEnd;
};
