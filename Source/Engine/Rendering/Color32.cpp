#include "Color32.h"

#include "Color.h"
#include "GMath.h"

Color32 Color32::Clear(0, 0, 0, 0);

Color32 Color32::Black(0, 0, 0, 255);
Color32 Color32::White(255, 255, 255, 255);
Color32 Color32::Gray(127, 127, 127, 255);

Color32 Color32::Red(255, 0, 0, 255);
Color32 Color32::Green(0, 255, 0, 255);
Color32 Color32::Blue(0, 0, 255, 255);

Color32 Color32::Cyan(0, 255, 255, 255);
Color32 Color32::Magenta(255, 0, 255, 255);
Color32 Color32::Yellow(255, 255, 0, 255);

Color32 Color32::Orange(255, 165, 0, 255);

Color32::Color32(uint8_t r, uint8_t g, uint8_t b) :
    r(r), g(g), b(b)
{

}

Color32::Color32(uint8_t r, uint8_t g, uint8_t b, uint8_t a) :
    r(r), g(g), b(b), a(a)
{

}

Color32::Color32(int r, int g, int b)
{
    this->r = static_cast<uint8_t>(Math::Clamp(r, 0, 255));
    this->g = static_cast<uint8_t>(Math::Clamp(g, 0, 255));
    this->b = static_cast<uint8_t>(Math::Clamp(b, 0, 255));
}

Color32::Color32(int r, int g, int b, int a)
{
    this->r = static_cast<uint8_t>(Math::Clamp(r, 0, 255));
    this->g = static_cast<uint8_t>(Math::Clamp(g, 0, 255));
    this->b = static_cast<uint8_t>(Math::Clamp(b, 0, 255));
    this->a = static_cast<uint8_t>(Math::Clamp(a, 0, 255));
}

Color32::Color32(const Color& other) : Color32(
    static_cast<int>(other.r * 255.0f),
    static_cast<int>(other.g * 255.0f),
    static_cast<int>(other.b * 255.0f),
    static_cast<int>(other.a * 255.0f))
{

}

bool Color32::operator==(const Color32& other) const
{
    return r == other.r &&
           g == other.g &&
           b == other.b &&
           a == other.a;
}

bool Color32::operator!=(const Color32& other) const
{
    return !(*this == other);
}

Color32 Color32::operator+(const Color32& other) const
{
    // Values are clamped 0-255 in Color32(int...) constructor.
    return Color32(r + other.r, g + other.g, b + other.b, a + other.a);
}

Color32& Color32::operator+=(const Color32& other)
{
    r = static_cast<uint8_t>(Math::Min(r + other.r, 255));
    g = static_cast<uint8_t>(Math::Min(g + other.g, 255));
    b = static_cast<uint8_t>(Math::Min(b + other.b, 255));
    a = static_cast<uint8_t>(Math::Min(a + other.a, 255));
    return *this;
}

Color32 Color32::operator-(const Color32& other) const
{
    // Values are clamped 0-255 in Color32(int...) constructor.
    return Color32(r - other.r, g - other.g, b - other.b, a - other.a);
}

Color32& Color32::operator-=(const Color32& other)
{
    r = static_cast<uint8_t>(Math::Max(r - other.r, 0));
    g = static_cast<uint8_t>(Math::Max(g - other.g, 0));
    b = static_cast<uint8_t>(Math::Max(b - other.b, 0));
    a = static_cast<uint8_t>(Math::Max(a - other.a, 0));
    return *this;
}

Color32 Color32::operator*(const Color32& other) const
{
    return Color32(r * other.r, g * other.g, b * other.b, a * other.a);
}

Color32& Color32::operator*=(const Color32& other)
{
    r *= other.r;
    g *= other.g;
    b *= other.b;
    a *= other.a;
    return *this;
}

/*static*/ Color32 Color32::Lerp(const Color32& from, const Color32& to, float t)
{
    Color32 result;
    result.r = Math::Lerp(from.r, to.r, t);
    result.g = Math::Lerp(from.g, to.g, t);
    result.b = Math::Lerp(from.b, to.b, t);
    result.a = Math::Lerp(from.a, to.a, t);
    return result;
}

std::ostream& operator<<(std::ostream& os, const Color32& c)
{
    os << "(" << static_cast<uint32_t>(c.r) << ", "
       <<        static_cast<uint32_t>(c.g) << ", "
       <<        static_cast<uint32_t>(c.b) << ", "
       <<        static_cast<uint32_t>(c.a) << ")";
    return os;
}
