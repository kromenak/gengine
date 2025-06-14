#include "AABB.h"

/*static*/ AABB AABB::FromMinMax(const Vector3& min, const Vector3& max)
{
    return AABB(min, max);
}

/*static*/ AABB AABB::FromPoints(const Vector3& pointA, const Vector3& pointB)
{
    // The difference here is that we don't know which point is the min or max.
    // But we can let GrowToContain figure it out for us!
    AABB aabb(pointA, pointA);
    aabb.GrowToContain(pointB);
    return aabb;
}

/*static*/ AABB AABB::FromCenterAndExtents(const Vector3& center, const Vector3& extents)
{
    AABB aabb;
    aabb.mMin = center - extents;
    aabb.mMax = center + extents;
    return aabb;
}

/*static*/ AABB AABB::FromCenterAndSize(const Vector3& center, const Vector3& size)
{
    return FromCenterAndExtents(center, size * 0.5f);
}

AABB::AABB(const Vector3& min, const Vector3& max) :
    mMin(min),
    mMax(max)
{

}

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
