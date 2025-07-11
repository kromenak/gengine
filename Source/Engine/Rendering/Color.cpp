#include "Color.h"

#include "Color32.h"
#include "GMath.h"

Color Color::Clear(0.0f, 0.0f, 0.0f, 0.0f);

Color Color::Black(0.0f, 0.0f, 0.0f, 1.0f);
Color Color::White(1.0f, 1.0f, 1.0f, 1.0f);
Color Color::Gray(0.5f, 0.5f, 0.5f, 1.0f);

Color Color::Red(1.0f, 0.0f, 0.0f, 1.0f);
Color Color::Green(0.0f, 1.0f, 0.0f, 1.0f);
Color Color::Blue(0.0f, 0.0f, 1.0f, 1.0f);

Color Color::Cyan(0.0f, 1.0f, 1.0f, 1.0f);
Color Color::Magenta(1.0f, 0.0f, 1.0f, 1.0f);
Color Color::Yellow(1.0f, 1.0f, 0.0f, 1.0f);

Color Color::Orange(1.0f, 0.65f, 0.0f, 1.0f);

Color::Color(float r, float g, float b) :
    r(r), g(g), b(b)
{

}

Color::Color(float r, float g, float b, float a) :
    r(r), g(g), b(b), a(a)
{

}

Color::Color(const Color32& other) :
    r(other.r / 255.0f),
    g(other.g / 255.0f),
    b(other.b / 255.0f),
    a(other.a / 255.0f)
{

}

bool Color::operator==(const Color& other) const
{
    return (Math::AreEqual(r, other.r) &&
            Math::AreEqual(g, other.g) &&
            Math::AreEqual(b, other.b) &&
            Math::AreEqual(a, other.a));
}

bool Color::operator!=(const Color& other) const
{
    return !(*this == other);
}

Color Color::operator+(const Color& other) const
{
    return Color(r + other.r, g + other.g, b + other.b, a + other.a);
}

Color& Color::operator+=(const Color& other)
{
    r += other.r;
    g += other.g;
    b += other.b;
    a += other.a;
    return *this;
}

Color Color::operator-(const Color& other) const
{
    return Color(r - other.r, g - other.g, b - other.b, a - other.a);
}

Color& Color::operator-=(const Color& other)
{
    r -= other.r;
    g -= other.g;
    b -= other.b;
    a -= other.a;
    return *this;
}

Color Color::operator*(const Color& other) const
{
    return Color(r * other.r, g * other.g, b * other.b, a * other.a);
}

Color& Color::operator*=(const Color& other)
{
    r *= other.r;
    g *= other.g;
    b *= other.b;
    a *= other.a;
    return *this;
}

/*static*/ Color Color::Lerp(const Color& from, const Color& to, float t)
{
    Color result;
    result.r = Math::Lerp(from.r, to.r, t);
    result.g = Math::Lerp(from.g, to.g, t);
    result.b = Math::Lerp(from.b, to.b, t);
    result.a = Math::Lerp(from.a, to.a, t);
    return result;
}
