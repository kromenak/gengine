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

class AABB
{
public:
    AABB() = default;
	AABB(const Vector3& min, const Vector3& max);
    AABB(const Vector3& center, float extentsX, float extentsY, float extentsZ);
	
	Vector3 GetMin() const { return mMin; }
	Vector3 GetMax() const { return mMax; }
	
	Vector3 GetCenter() const { return mMin + ((mMax - mMin) * 0.5f); }
	Vector3 GetExtents() const { return ((mMax - mMin) * 0.5f); }
	
	void GrowToContain(const Vector3& point);
	
	bool IsValid() const { return mMin.x <= mMax.x && mMin.y <= mMax.y && mMin.z <= mMax.z; }
	
	bool ContainsPoint(const Vector3& point) const;
	Vector3 GetClosestPoint(const Vector3& point) const;
	
private:
    // Min and max points of the AABB.
    // Keep private b/c AABB can be represented as min/max points or center/size...may want or need to switch this at some point.
	Vector3 mMin;
	Vector3 mMax;
};
