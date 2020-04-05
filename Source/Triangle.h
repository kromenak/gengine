//
// Triangle.h
//
// Clark Kromenaker
//
// A triangle is three points that are not collinear.
// Not totally sure if a triangle class is necessary. But it might come in handy.
//
#pragma once
#include "Vector3.h"

class Triangle
{
public:
	Triangle(const Vector3& p0, const Vector3& p1, const Vector3& p2);
	
	bool ContainsPoint(const Vector3& point) const;
	Vector3 GetClosestPoint(const Vector3& point) const;
	
	// Triangle tests may need to be done on hundreds or thousands of triangles per frame.
	// It may be desirable to utilize triangle algorithms without overhead of creating a triangle instance.
	static bool ContainsPoint(const Vector3& p0, const Vector3& p1, const Vector3& p2, const Vector3& point);
	static Vector3 GetClosestPoint(const Vector3& p0, const Vector3& p1, const Vector3& p2, const Vector3& point);
	
private:
	// A triangle consists of three points!
	Vector3 mP0;
	Vector3 mP1;
	Vector3 mP2;
};
