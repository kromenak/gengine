//
// Clark Kromenaker
//
// Functions related to interpolating between two values in various ways.
//
#pragma once

namespace Interpolate
{
    template<typename T>
    [[nodiscard]] inline constexpr T Linear(T a, T b, float t)
    {
        return a + ((b - a) * t);
    }

    namespace Internal
    {
        inline constexpr float Flip(float t) { return 1.0f - t; }

        inline constexpr float QuadraticIn(float t) { return t * t; }
        inline constexpr float QuadraticOut(float t) { return Flip(QuadraticIn(Flip(t))); }
        inline constexpr float QuadraticInOut(float t) { return Linear(QuadraticIn(t), QuadraticOut(t), t); }

        inline constexpr float CubicIn(float t) { return t * t * t; }
        inline constexpr float CubicOut(float t) { return Flip(CubicIn(Flip(t))); }
        inline constexpr float CubicInOut(float t) { return Linear(CubicIn(t), CubicOut(t), t); }
    }

    template<typename T>
    [[nodiscard]] inline constexpr T QuadraticIn(T a, T b, float t)
    {
        return Linear(a, b, Internal::QuadraticIn(t));
    }

    template<typename T>
    [[nodiscard]] inline constexpr T QuadraticOut(T a, T b, float t)
    {
        return Linear(a, b, Internal::QuadraticOut(t));
    }

    template<typename T>
    [[nodiscard]] inline constexpr T QuadraticInOut(T a, T b, float t)
    {
        return Linear(a, b, Internal::QuadraticInOut(t));
    }

    template<typename T>
    [[nodiscard]] inline constexpr T CubicIn(T a, T b, float t)
    {
        return Linear(a, b, Internal::CubicIn(t));
    }

    template<typename T>
    [[nodiscard]] inline constexpr T CubicOut(T a, T b, float t)
    {
        return Linear(a, b, Internal::CubicOut(t));
    }

    template<typename T>
    [[nodiscard]] inline constexpr T CubicInOut(T a, T b, float t)
    {
        return Linear(a, b, Internal::CubicInOut(t));
    }
}