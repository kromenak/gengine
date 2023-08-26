#include "Color32.h"

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

Color32::Color32(unsigned char r, unsigned char g, unsigned char b) :
	r(r), g(g), b(b)
{
	
}

Color32::Color32(unsigned char r, unsigned char g, unsigned char b, unsigned char a) :
	r(r), g(g), b(b), a(a)
{
	
}

Color32::Color32(int r, int g, int b)
{
	this->r = (unsigned char)Math::Clamp(r, 0, 255);
	this->g = (unsigned char)Math::Clamp(g, 0, 255);
	this->b = (unsigned char)Math::Clamp(b, 0, 255);
}

Color32::Color32(int r, int g, int b, int a)
{
	this->r = (unsigned char)Math::Clamp(r, 0, 255);
	this->g = (unsigned char)Math::Clamp(g, 0, 255);
	this->b = (unsigned char)Math::Clamp(b, 0, 255);
	this->a = (unsigned char)Math::Clamp(a, 0, 255);
}

bool Color32::operator==(const Color32& other) const
{
	return (Math::AreEqual(r, other.r) &&
			Math::AreEqual(g, other.g) &&
			Math::AreEqual(b, other.b) &&
			Math::AreEqual(a, other.a));
}

bool Color32::operator!=(const Color32& other) const
{
	return !(Math::AreEqual(r, other.r) &&
			 Math::AreEqual(g, other.g) &&
			 Math::AreEqual(b, other.b) &&
			 Math::AreEqual(a, other.a));
}

Color32 Color32::operator+(const Color32& other) const
{
	return Color32(r + other.r, g + other.g, b + other.b, a + other.a);
}

Color32& Color32::operator+=(const Color32& other)
{
	r = (unsigned char)Math::Min(r + other.r, 255);
	g = (unsigned char)Math::Min(g + other.g, 255);
	b = (unsigned char)Math::Min(b + other.b, 255);
	a = (unsigned char)Math::Min(a + other.a, 255);
	return *this;
}

Color32 Color32::operator-(const Color32& other) const
{
	return Color32(r - other.r, g - other.g, b - other.b, a - other.a);
}

Color32& Color32::operator-=(const Color32& other)
{
	r = (unsigned char)Math::Max(r - other.r, 0);
	g = (unsigned char)Math::Max(g - other.g, 0);
	b = (unsigned char)Math::Max(b - other.b, 0);
	a = (unsigned char)Math::Max(a - other.a, 0);
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
	os << "(" << (unsigned int)c.GetR() << ", " << (unsigned int)c.GetG()
	<< ", " << (unsigned int)c.GetB() << ", " << (unsigned int)c.GetA() << ")";
	return os;
}
