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
#include <cstdint>
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

    [[nodiscard]] inline constexpr float Abs(float val)
    {
        // Can't use std::abs here because it isn't constexpr until C++23!
        return val < 0.0f ? -val : val;
    }

    [[nodiscard]] inline float Sqrt(float val)
    {
        return std::sqrt(val);
    }

    [[nodiscard]] inline float InvSqrt(float val)
    {
        return 1.0f / std::sqrt(val);
    }

    [[nodiscard]] inline constexpr bool IsZero(float val)
    {
        return Abs(val) < kEpsilon;
    }

    [[nodiscard]] inline constexpr bool AreEqual(float a, float b)
    {
        return IsZero(a - b);
    }

    [[nodiscard]] inline constexpr bool Approximately(float a, float b, float epsilon = kEpsilon)
    {
        return Abs(a - b) < epsilon;
    }

    [[nodiscard]] inline float Pow(float base, float exp)
    {
        return std::pow(base, exp);
    }

    [[nodiscard]] inline constexpr int PowBase2(int exp)
    {
        return 1 << exp;
    }

    [[nodiscard]] inline float Log(float val)
    {
        return std::log(val);
    }

    [[nodiscard]] inline float LogBase2(float val)
    {
        return std::log2(val);
    }

    [[nodiscard]] inline float Mod(float num1, float num2)
    {
        // floating-point equivalent of "return num1 % num2;"
        return std::fmod(num1, num2);
    }

    [[nodiscard]] inline float Sin(float radians)
    {
        return std::sin(radians);
    }

    [[nodiscard]] inline float Asin(float ratio)
    {
        // See comment in acos below.
        return std::asin(std::clamp(ratio, -1.0f, 1.0f));
    }

    [[nodiscard]] inline float Cos(float radians)
    {
        return std::cos(radians);
    }

    [[nodiscard]] inline float Acos(float ratio)
    {
        // Even if passed in ratio should be in valid range (e.g. dot product of two normalized vectors),
        // There's a chance it'll be *just* outside that range, due to floating point imprecision.
        // It's safer to clamp the range here than to allow NaN to propagate in the program!
        return std::acos(std::clamp(ratio, -1.0f, 1.0f));
    }

    [[nodiscard]] inline float Tan(float radians)
    {
        return std::tan(radians);
    }

    [[nodiscard]] inline float Atan(float ratio)
    {
        return std::atan(ratio);
    }

    [[nodiscard]] inline float Atan2(float y, float x)
    {
        return std::atan2(y, x);
    }

    [[nodiscard]] inline constexpr float Floor(float val)
    {
        // std::floor isn't constexpr until C++23.
        // For positive numbers, truncating would be sufficient. But we have to deal with negatives.
        // For negative numbers, val ends up being less than the truncated value, and must subtract one (e.g. -4.7 floored is -5).
        const int64_t truncated = static_cast<int64_t>(val);
        const float f = static_cast<float>(truncated);
        return (val < f) ? f - 1.0f : f;
    }

    [[nodiscard]] inline constexpr float Ceil(float val)
    {
        // std::ceil isn't constexpr until C++23.
        // Similar to Floor, but inversed. Negative numbers are easy, but positive numbers need extra work.
        const int64_t truncated = static_cast<int64_t>(val);
        const float f = static_cast<float>(truncated);
        return (val > f) ? f + 1.0f : f;
    }

    [[nodiscard]] inline constexpr float Round(float val)
    {
        // std::round isn't constexpr until C++23.
        // For positive values, adding 0.5f and flooring ensures that anything 0.5f and up rounds up, anything below rounds down.
        // For negative values, subtract 0.5f and ceil ensures anything -0.5f and below rounds down and anything 0.5f and up rounds up.
        return (val < 0.0f) ? Ceil(val - 0.5f) : Floor(val + 0.5f);
    }

    [[nodiscard]] inline constexpr int FloorToInt(float val)
    {
        return static_cast<int>(Floor(val));
    }

    [[nodiscard]] inline constexpr int CeilToInt(float val)
    {
        return static_cast<int>(Ceil(val));
    }

    [[nodiscard]] inline constexpr int RoundToInt(float val)
    {
        return static_cast<int>(Round(val));
    }

    [[nodiscard]] inline constexpr float Truncate(float val)
    {
        return static_cast<float>(static_cast<int64_t>(val));
    }

    [[nodiscard]] inline constexpr int TruncateToInt(float val)
    {
        return static_cast<int>(val);
    }

    [[nodiscard]] inline constexpr float MagnitudeSign(float mag, float sign)
    {
        // std::copysign isn't constexpr until C++23.
        // Take magnitude of first number and sign of second number.
        // Return product of those two things. (e.g. 35, -18 => -35)
        float absMag = Abs(mag);
        return (sign < 0.0f) ? -absMag : absMag;
    }

    template<typename T>
    [[nodiscard]] inline constexpr T Min(T val1, T val2)
    {
        return val1 < val2 ? val1 : val2;
    }

    template<typename T>
    [[nodiscard]] inline constexpr T Max(T val1, T val2)
    {
        return val1 > val2 ? val1 : val2;
    }

    template<typename T>
    [[nodiscard]] inline constexpr T Clamp(T value, T min, T max)
    {
        return Min(max, Max(value, min));
    }

    [[nodiscard]] inline constexpr float ToDegrees(float radians)
    {
        return (radians * (180.0f / kPi));
    }

    [[nodiscard]] inline constexpr float ToRadians(float degrees)
    {
        return (degrees * (kPi / 180.0f));
    }

    template<typename T>
    [[nodiscard]] inline T Lerp(T a, T b, float t)
    {
        return Interpolate::Linear(a, b, t);
    }
}