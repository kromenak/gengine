//
// Vector2.cpp
//
// Clark Kromenaker
//
#include "Vector2.h"

#include "Vector3.h"

Vector2 Vector2::Zero(0.0f, 0.0f);
Vector2 Vector2::One(1.0f, 1.0f);
Vector2 Vector2::UnitX(1.0f, 0.0f);
Vector2 Vector2::UnitY(0.0f, 1.0f);

Vector2::Vector2() : Vector2(0.0f, 0.0f)
{
    
}

Vector2::Vector2(float x, float y) : x(x), y(y)
{
    
}

Vector2::Vector2(const Vector2& other) : x(other.x), y(other.y)
{
    
}

Vector2::Vector2(const Vector3& other) : x(other.GetX()), y(other.GetY())
{
	
}

Vector2& Vector2::operator=(const Vector2& other)
{
    x = other.x;
    y = other.y;
    return *this;
}

bool Vector2::operator==(const Vector2& other) const
{
    return (Math::AreEqual(x, other.x) &&
            Math::AreEqual(y, other.y));
}

bool Vector2::operator!=(const Vector2& other) const
{
    return !(Math::AreEqual(x, other.x) &&
             Math::AreEqual(y, other.y));
}

Vector2& Vector2::Normalize()
{
    // Get length squared. If zero, it means we can't normalize!
    float lengthSq = GetLengthSq();
    if(Math::IsZero(lengthSq))
    {
        return *this;
    }
    
    // Normalization is each component divided by length of vector.
    // We use this method because inverse square root could be faster.
    // oneOverLength = 1 / ||v||
    float oneOverLength = Math::InvSqrt(lengthSq);
    
    // Multiply each component by this, which is equal to divide by length.
    x *= oneOverLength;
    y *= oneOverLength;
    return *this;
}

Vector2 Vector2::operator+(const Vector2 &other) const
{
    return Vector2(x + other.x, y + other.y);
}

Vector2& Vector2::operator+=(const Vector2 &other)
{
    x += other.x;
    y += other.y;
    return *this;
}

Vector2 Vector2::operator-(const Vector2 &other) const
{
    return Vector2(x - other.x, y - other.y);
}

Vector2& Vector2::operator-=(const Vector2 &other)
{
    x -= other.x;
    y -= other.y;
    return *this;
}

Vector2 Vector2::operator-() const
{
    return Vector2(-x, -y);
}

Vector2 Vector2::operator*(float scalar) const
{
    return Vector2(x * scalar, y * scalar);
}

Vector2 operator*(float scalar, const Vector2& vector)
{
    return Vector2(vector.x * scalar, vector.y * scalar);
}

Vector2& Vector2::operator*=(float scalar)
{
    x *= scalar;
    y *= scalar;
    return *this;
}

Vector2 Vector2::operator/(float scalar) const
{
    return Vector2(x / scalar, y / scalar);
}

Vector2& Vector2::operator/=(float scalar)
{
    x /= scalar;
    y /= scalar;
    return *this;
}

float Vector2::Dot(Vector2 lhs, Vector2 rhs)
{
    return (lhs.x * rhs.x + lhs.y * rhs.y);
}

Vector2 Vector2::Lerp(Vector2 from, Vector2 to, float t)
{
	return ((1.0f - t) * from) + (t * to);
}

std::ostream& operator<<(std::ostream& os, const Vector2& v)
{
    os << "(" << v.GetX() << ", " << v.GetY() << ")";
    return os;
}
