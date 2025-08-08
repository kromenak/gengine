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
#include <cstdlib>

#include "Interpolate.h"

namespace Math
{
    // Floating-point numbers within 0.000001 units are considered equal to one another.
    static constexpr float kEpsilon = 1.0e-6f;

    // Pi constants.
    static constexpr float kPi = 3.1415926535897932384626433832795f;
    static constexpr float k2Pi = 2.0f * kPi;
    static constexpr float kPiOver2 = kPi / 2.0f;
    static constexpr float kPiOver4 = kPi / 4.0f;

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

    inline float Log(float val)
    {
        return std::log(val);
    }

    inline float LogBase2(float val)
    {
        return std::log2(val);
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
        // See comment in acos below.
        return ::asinf(std::fmin(1.0f, std::fmax(ratio, -1.0f)));
    }

    inline float Cos(float radians)
    {
        return ::cosf(radians);
    }

    inline float Acos(float ratio)
    {
        // Even if passed in ratio should be in valid range (e.g. dot product of two normalized vectors),
        // There's a chance it'll be *just* outside that range, due to floating point imprecision.
        // It's safer to clamp the range here than to allow NaN to propagate in the program!
        return ::acosf(std::fmin(1.0f, std::fmax(ratio, -1.0f)));
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

    inline float Truncate(float val)
    {
        return static_cast<float>(static_cast<int>(val));
    }

    inline float TruncateToInt(float val)
    {
        return static_cast<int>(val);
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

    template<typename T>
    inline T Min(T val1, T val2)
    {
        return val1 < val2 ? val1 : val2;
    }

    template<typename T>
    inline T Max(T val1, T val2)
    {
        return val1 > val2 ? val1 : val2;
    }

    template<typename T>
    inline T Clamp(T value, T min, T max)
    {
        return Min(max, Max(value, min));
    }

    inline constexpr float ToDegrees(float radians)
    {
        return (radians * (180.0f / kPi));
    }

    inline constexpr float ToRadians(float degrees)
    {
        return (degrees * (kPi / 180.0f));
    }

    template<typename T>
    inline T Lerp(T a, T b, float t)
    {
        return Interpolate::Linear(a, b, t);
    }
}