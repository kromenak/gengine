//
// Plane.cpp
//
// Clark Kromenaker
//
#include "Plane.h"

Plane::Plane() : mNormal(1.0f, 0.0f, 0.0f), mDistance(0.0f)
{
    
}

Plane::Plane(float normalX, float normalY, float normalZ, float distance)
{
    Set(normalX, normalY, normalZ, distance);
}

Vector3 Plane::FindClosestPointOnPlane(const Vector3& point) const
{
    return point - Test(point) * mNormal;
}

void Plane::Set(float x, float y, float z, float d)
{
    float lengthSq = x * x + y * y + z * z;
    if(Math::IsZero(lengthSq))
    {
        mNormal = Vector3::UnitX;
        mDistance = 0.0f;
    }
    else
    {
        float recip = Math::InvSqrt(lengthSq);
        mNormal.SetX(x * recip);
        mNormal.SetY(y * recip);
        mNormal.SetZ(z * recip);
        mDistance = d * recip;
    }
}
