//
// Clark Kromenaker
//
// General purpose math functions.
//
// NOTE: Called "GMath" to avoid potential name conflict on Windows.
// NOTE: Using global functions (e.g. ::sqrtf instead of std::sqrtf) due to non-conformant C++11 compiler on Linux.
//
#pragma once
#include <algorithm>
#include <cmath>

namespace Math
{
    // Floating-point numbers within 0.000001 units are considered equal to one another.
    static const float kEpsilon = 1.0e-6f;
    
    // Pi constants.
    static const float kPi = 3.1415926535897932384626433832795f;
    static const float k2Pi = 2.0f * kPi;
    static const float kPiOver2 = kPi / 2.0f;
    static const float kPiOver4 = kPi / 4.0f;
    
    inline float Sqrt(float val)
    {
        return ::sqrtf(val);
    }
    
    inline float InvSqrt(float val)
    {
        //TODO: this could be replaced by a faster (but approximate) calculation
        //TODO: the famous "fast inverse square root!"
        //TODO: https://www.slideshare.net/maksym_zavershynskyi/fast-inverse-square-root
        return (1.0f / ::sqrtf(val));
    }
    
    inline bool IsZero(float val)
    {
		return (::fabsf(val) < kEpsilon);
    }
    
    inline bool AreEqual(float a, float b)
    {
        return IsZero(a - b);
    }

    inline bool Approximately(float a, float b, float epsilon = kEpsilon)
    {
        return (::fabs(a - b) < epsilon);
    }
	
	inline float Pow(float base, float exp)
	{
		return std::pow(base, exp);
	}
	
	inline int PowBase2(int exp)
	{
		return 1 << exp;
	}
    
    inline float Mod(float num1, float num2)
    {
		// floating-point equivalent of "return num1 % num2;"
		return std::fmod(num1, num2);
    }
    
    inline float Sin(float radians)
    {
        return ::sinf(radians);
    }
    
    inline float Asin(float ratio)
    {
        return ::asinf(ratio);
    }
    
    inline float Cos(float radians)
    {
        return ::cosf(radians);
    }
    
    inline float Acos(float ratio)
    {
        return ::acosf(ratio);
    }
    
    inline float Tan(float radians)
    {
        return ::tanf(radians);
    }
    
    inline float Atan(float ratio)
    {
        return ::atanf(ratio);
    }

	inline float Atan2(float y, float x)
	{
		return std::atan2(y, x);
	}
    
    inline float Floor(float val)
    {
        return std::floor(val);
    }
    
    inline float Ceil(float val)
    {
        return std::ceil(val);
    }
    
    inline float Round(float val)
    {
        return std::round(val);
    }
    
    inline int FloorToInt(float val)
    {
        return static_cast<int>(Floor(val));
    }
    
    inline int CeilToInt(float val)
    {
        return static_cast<int>(Ceil(val));
    }
    
    inline int RoundToInt(float val)
    {
        return static_cast<int>(Round(val));
    }
    
    inline float Clamp(float value, float min, float max)
    {
        return std::fmin(max, std::fmax(value, min));
    }
    
    inline int Clamp(int value, int min, int max)
    {
        return std::min(max, std::max(value, min));
    }
    
    inline float Min(float val1, float val2)
    {
        return std::fmin(val1, val2);
    }
    
    inline int Min(int val1, int val2)
    {
        return std::min(val1, val2);
    }
    
    inline float Max(float val1, float val2)
    {
        return std::fmax(val1, val2);
    }
    
    inline int Max(int val1, int val2)
    {
        return std::max(val1, val2);
    }
    
    inline float Abs(float val)
    {
        return std::abs(val);
    }

	inline float MagnitudeSign(float mag, float sign)
	{
        // Take magnitude of first number and sign of second number.
        // Return product of those two things. (e.g. 35, -18 => -35)
		return std::copysign(mag, sign);
	}
    
    inline float ToDegrees(float radians)
    {
        return (radians * (180.0f / kPi));
    }
    
    inline float ToRadians(float degrees)
    {
        return (degrees * (kPi / 180.0f));
    }
	
	inline float Lerp(float a, float b, float t)
	{
		return a + ((b - a) * t);
	}
	
	inline unsigned char Lerp(unsigned char a, unsigned char b, float t)
	{
		return static_cast<unsigned char>(a + ((b - a) * t));
	}
}
