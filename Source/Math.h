//
//  Math.h
//  GEngine
//
//  Created by Clark Kromenaker on 10/16/17.
//
#pragma once
#include <cmath>

namespace Math
{
    // Dictates that floating-point numbers within 0.000001 units are equal to one another.
    static const float kEpsilon = 1.0e-6f;
    //TODO: PI Stuff
    
    // Calculates the square root of a value.
    inline float Sqrt(float val)
    {
        return std::sqrtf(val);
    }
    
    // Calculates the inverse square root of a value.
    inline float InvSqrt(float val)
    {
        return (1.0f / std::sqrtf(val));
    }
    
    inline bool IsZero(float val)
    {
        return (fabsf(val) < kEpsilon);
    }
    
    inline bool AreEqual(float a, float b)
    {
        return IsZero(a - b);
    }
}
