//
// Vector4.cpp
//
// Clark Kromenaker
// 
#include "Vector4.h"

Vector4 Vector4::Zero(0.0f, 0.0f, 0.0f, 0.0f);
Vector4 Vector4::One(1.0f, 1.0f, 1.0f, 1.0f);
Vector4 Vector4::UnitX(1.0f, 0.0f, 0.0f, 0.0f);
Vector4 Vector4::UnitY(0.0f, 1.0f, 0.0f, 0.0f);
Vector4 Vector4::UnitZ(0.0f, 0.0f, 1.0f, 0.0f);
Vector4 Vector4::UnitW(0.0f, 0.0f, 0.0f, 1.0f);

Vector4::Vector4() : Vector4(0.0f, 0.0f, 0.0f, 0.0f)
{
    
}

Vector4::Vector4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w)
{
    
}

Vector4::Vector4(bool isPos) : Vector4(0.0f, 0.0f, 0.0f, 0.0f)
{
    w = isPos ? 1.0f : 0.0f;
}

Vector4::Vector4(const Vector4& other) : x(other.x), y(other.y), z(other.z), w(other.w)
{
    
}

Vector4& Vector4::operator=(const Vector4& other)
{
    x = other.x;
    y = other.y;
    z = other.z;
    w = other.w;
    return *this;
}

bool Vector4::operator==(const Vector4& other) const
{
    return (Math::AreEqual(x, other.x) &&
            Math::AreEqual(y, other.y) &&
            Math::AreEqual(z, other.z) &&
            Math::AreEqual(w, other.w));
}

bool Vector4::operator!=(const Vector4& other) const
{
    return !(Math::AreEqual(x, other.x) &&
             Math::AreEqual(y, other.y) &&
             Math::AreEqual(z, other.z) &&
             Math::AreEqual(w, other.w));
}

Vector4& Vector4::Normalize()
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
    z *= oneOverLength;
    w *= oneOverLength;
    return *this;
}

Vector4 Vector4::operator+(const Vector4 &other) const
{
    return Vector4(x + other.x, y + other.y, z + other.z, w + other.w);
}

Vector4& Vector4::operator+=(const Vector4 &other)
{
    x += other.x;
    y += other.y;
    z += other.z;
    w += other.w;
    return *this;
}

Vector4 Vector4::operator-(const Vector4 &other) const
{
    return Vector4(x - other.x, y - other.y, z - other.z, w - other.w);
}

Vector4& Vector4::operator-=(const Vector4 &other)
{
    x -= other.x;
    y -= other.y;
    z -= other.z;
    w -= other.w;
    return *this;
}

Vector4 Vector4::operator-() const
{
    return Vector4(-x, -y, -z, -w);
}

Vector4 Vector4::operator*(float scalar) const
{
    return Vector4(x * scalar, y * scalar, z * scalar, w * scalar);
}

Vector4 operator*(float scalar, const Vector4& vector)
{
    return Vector4(vector.x * scalar, vector.y * scalar, vector.z * scalar, vector.w * scalar);
}

Vector4& Vector4::operator*=(float scalar)
{
    x *= scalar;
    y *= scalar;
    z *= scalar;
    w *= scalar;
    return *this;
}

Vector4 Vector4::operator/(float scalar) const
{
    return Vector4(x / scalar, y / scalar, z / scalar, w / scalar);
}

Vector4& Vector4::operator/=(float scalar)
{
    x /= scalar;
    y /= scalar;
    z /= scalar;
    w /= scalar;
    return *this;
}

Vector4 Vector4::operator*(const Vector4& other) const
{
	return Vector4(x * other.x, y * other.y, z * other.z, w * other.w);
}

float Vector4::Dot(Vector4 lhs, Vector4 rhs)
{
    return (lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z + lhs.w * rhs.w);
}

Vector4 Vector4::Cross(Vector4 lhs, Vector4 rhs)
{
    // Since Cross product isn't technically defined for Vector4,
    // we'll do the Vector4 operation and leave the w component set to 1.
    Vector4 result(lhs.y * rhs.z - lhs.z * rhs.y,
                   lhs.z * rhs.x - lhs.x * rhs.z,
                   lhs.x * rhs.y - lhs.y * rhs.x,
                   1.0f);
    return result;
}

std::ostream& operator<<(std::ostream& os, const Vector4& v)
{
    os << "(" << v.GetX() << ", " << v.GetY() << ", " << v.GetZ() << ", " << v.GetW() << ")";
    return os;
}
