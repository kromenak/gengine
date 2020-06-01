//
// AABB.cpp
//
// Clark Kromenaker
//
#include "AABB.h"

#include "Debug.h"

AABB::AABB(Vector3 min, Vector3 max)
{
	mMin = min;
	mMax = max;
}

/*
AABB::AABB(Vector3 center, float extentX, float extentY, float extentZ)
{
	Vector3 extent(extentX, extentY, extentZ);
	mMin = center - extent;
	mMax = center + extent;
}
*/

void AABB::GrowToContain(const Vector3& point)
{
	// Grow min.
	mMin.x = Math::Min(mMin.x, point.x);
	mMin.y = Math::Min(mMin.y, point.y);
	mMin.z = Math::Min(mMin.z, point.z);
	
	// Grow max.
	mMax.x = Math::Max(mMax.x, point.x);
	mMax.y = Math::Max(mMax.y, point.y);
	mMax.z = Math::Max(mMax.z, point.z);
}

bool AABB::ContainsPoint(const Vector3& point) const
{
	// Point should be greater than min and less than max.
	return point.x >= mMin.x && point.y >= mMin.y && point.z >= mMin.z &&
		   point.x <= mMax.x && point.y <= mMax.y && point.z <= mMax.z;
}

Vector3 AABB::GetClosestPoint(const Vector3& point) const
{
	// Start with result as point.
	// If point is inside AABB, we just return the ummodified point in the end.
	Vector3 result = point;
	
	// Clamp point between min/max on x-axis.
	if(point.x > mMax.x)
	{
		result.x = mMax.x;
	}
	else if(point.x < mMin.x)
	{
		result.x = mMin.x;
	}
	
	// Clamp point between min/max on y-axis.
	if(point.y > mMax.y)
	{
		result.y = mMax.y;
	}
	else if(point.y < mMin.y)
	{
		result.y = mMin.y;
	}
	
	// Clamp point between min/max on z-axis.
	if(point.z > mMax.z)
	{
		result.z = mMax.z;
	}
	else if(point.z < mMin.z)
	{
		result.z = mMin.z;
	}
	return result;
}

void AABB::DebugDraw(const Color32& color, float duration, const Matrix4* transformMatrix) const
{
	// Left side of box.
	Vector3 p0(mMin.x, mMin.y, mMin.z);
	Vector3 p1(mMin.x, mMin.y, mMax.z);
	Vector3 p3(mMin.x, mMax.y, mMin.z);
	Vector3 p2(mMin.x, mMax.y, mMax.z);
	
	// Right side of box.
	Vector3 p4(mMax.x, mMin.y, mMin.z);
	Vector3 p5(mMax.x, mMin.y, mMax.z);
	Vector3 p7(mMax.x, mMax.y, mMin.z);
	Vector3 p6(mMax.x, mMax.y, mMax.z);
	
	// After creating points, we may want to transform from some local space to world space before debug drawing.
	if(transformMatrix != nullptr)
	{
		p0 = transformMatrix->TransformPoint(p0);
		p1 = transformMatrix->TransformPoint(p1);
		p2 = transformMatrix->TransformPoint(p2);
		p3 = transformMatrix->TransformPoint(p3);
		p4 = transformMatrix->TransformPoint(p4);
		p5 = transformMatrix->TransformPoint(p5);
		p6 = transformMatrix->TransformPoint(p6);
		p7 = transformMatrix->TransformPoint(p7);
	}
	
	// Draw the lines of the box.
	Debug::DrawLine(p0, p1, color, duration);
	Debug::DrawLine(p1, p2, color, duration);
	Debug::DrawLine(p2, p3, color, duration);
	Debug::DrawLine(p3, p0, color, duration);
	
	Debug::DrawLine(p4, p5, color, duration);
	Debug::DrawLine(p5, p6, color, duration);
	Debug::DrawLine(p6, p7, color, duration);
	Debug::DrawLine(p7, p4, color, duration);
	
	Debug::DrawLine(p0, p4, color, duration);
	Debug::DrawLine(p1, p5, color, duration);
	Debug::DrawLine(p2, p6, color, duration);
	Debug::DrawLine(p3, p7, color, duration);
}
