//
// Clark Kromenaker
//
// A 32-bit RGBA color (1 byte per component)
//
#pragma once
#include <cstdint>
#include <iostream>

class Color;

class Color32
{
public:
    static Color32 Clear;   // (0, 0, 0, 0)

    static Color32 Black; 	// (0, 0, 0, 255)
    static Color32 White; 	// (255, 255, 255, 255)
    static Color32 Gray;    // (127, 127, 127, 255)

    static Color32 Red;	  	// (255, 0, 0, 255)
    static Color32 Green; 	// (0, 255, 0, 255)
    static Color32 Blue;  	// (0, 0, 255, 255)

    static Color32 Cyan;	// (0, 255, 255, 255)
    static Color32 Magenta; // (255, 0, 255, 255)
    static Color32 Yellow;  // (255, 255, 0, 255)

    static Color32 Orange;  // (255, 165, 0, 255)

    Color32() = default;
    Color32(uint8_t r, uint8_t g, uint8_t b);
    Color32(uint8_t r, uint8_t g, uint8_t b, uint8_t a);

    Color32(int r, int g, int b);
    Color32(int r, int g, int b, int a);

    // Conversion from Color
    Color32(const Color& other);

    // Equality
    bool operator==(const Color32& other) const;
    bool operator!=(const Color32& other) const;

    // Accessors
    uint8_t& operator[](uint32_t i)       { return (&r)[i]; }
    uint8_t  operator[](uint32_t i) const { return (&r)[i]; }

    // Addition/Subtraction
    Color32 operator+(const Color32& other) const;
    Color32& operator+=(const Color32& other);

    Color32 operator-(const Color32& other) const;
    Color32& operator-=(const Color32& other);

    // Multiplication
    Color32 operator*(const Color32& other) const;
    Color32& operator*=(const Color32& other);

    // Lerp
    static Color32 Lerp(const Color32& from, const Color32& to, float t);

    // Color components, valued 0-255 each.
    // Order is important for memory layout!
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
    uint8_t a = 255;
};

std::ostream& operator<<(std::ostream& os, const Color32& v);
