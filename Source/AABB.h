//
// AABB.h
//
// Clark Kromenaker
//
// An "axis-aligned bounding box." A simple rectangular bounding volume
// where the edges are always aligned to the coordinate axes.
//
#pragma once
#include "Vector3.h"

class Color32;
class Matrix4;

class AABB
{
public:
	AABB() { }
	AABB(Vector3 min, Vector3 max);
	//AABB(Vector3 center, float extentX, float extentY, float extentZ);
	
	Vector3 GetMin() const { return mMin; }
	Vector3 GetMax() const { return mMax; }
	
	Vector3 GetCenter() const { return mMin + ((mMax - mMin) * 0.5f); }
	Vector3 GetExtents() const { return ((mMax - mMin) * 0.5f); }
	
	void GrowToContain(const Vector3& point);
	
	bool IsValid() const { return mMin.x <= mMax.x && mMin.y <= mMax.y && mMin.z <= mMax.z; }
	
	bool ContainsPoint(const Vector3& point) const;
	Vector3 GetClosestPoint(const Vector3& point) const;
	
	void DebugDraw(const Color32& color, float duration = 0.0f, const Matrix4* transformMatrix = nullptr) const;
	
private:
	Vector3 mMin;
	Vector3 mMax;
};
