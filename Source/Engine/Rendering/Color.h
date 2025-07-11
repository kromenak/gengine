//
// Clark Kromenaker
//
// A color expressed as percentages of red/green/blue/alpha (where 0.0f is 0% and 1.0f is 100%).
//
// Differs from Color32 because it stores colors values as floats instead of uint8.
// Also, doesn't clamp color values 0.0f-1.0f, so modifying colors can result in components outside that range.
//
#pragma once
#include <cstdint>

class Color32;

class Color
{
public:
    static Color Clear;     // (0, 0, 0, 0)

    static Color Black;     // (0, 0, 0, 1)
    static Color White;     // (1, 1, 1, 1)
    static Color Gray;      // (0.5, 0.5, 0.5, 1)

    static Color Red;       // (1, 0, 0, 1)
    static Color Green;     // (0, 1, 0, 1)
    static Color Blue;      // (0, 0, 1, 1)

    static Color Cyan;      // (0, 1, 1, 1)
    static Color Magenta;   // (1, 0, 1, 1)
    static Color Yellow;    // (1, 1, 0, 1)

    static Color Orange;    // (1, 0.65, 0, 1)

    Color() = default;
    Color(float r, float g, float b);
    Color(float r, float g, float b, float a);

    // Conversion from Color32
    Color(const Color32& other);

    // Equality
    bool operator==(const Color& other) const;
    bool operator!=(const Color& other) const;

    // Accessors
    float& operator[](uint32_t i) { return (&r)[i]; }
    float  operator[](uint32_t i) const { return (&r)[i]; }

    // Addition/Subtraction
    Color operator+(const Color& other) const;
    Color& operator+=(const Color& other);

    Color operator-(const Color& other) const;
    Color& operator-=(const Color& other);

    // Multiplication
    Color operator*(const Color& other) const;
    Color& operator*=(const Color& other);

    // Lerp
    static Color Lerp(const Color& from, const Color& to, float t);

    // Color components, valued 0.0f-1.0f each.
    // Order is important for memory layout!
    float r = 0.0f;
    float g = 0.0f;
    float b = 0.0f;
    float a = 1.0f;
};