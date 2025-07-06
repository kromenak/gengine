#include "Vector4.h"

#include "Vector3.h"

Vector4 Vector4::Zero(0.0f, 0.0f, 0.0f, 0.0f);
Vector4 Vector4::One(1.0f, 1.0f, 1.0f, 1.0f);
Vector4 Vector4::UnitX(1.0f, 0.0f, 0.0f, 0.0f);
Vector4 Vector4::UnitY(0.0f, 1.0f, 0.0f, 0.0f);
Vector4 Vector4::UnitZ(0.0f, 0.0f, 1.0f, 0.0f);
Vector4 Vector4::UnitW(0.0f, 0.0f, 0.0f, 1.0f);

Vector4::Vector4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w)
{

}

Vector4::Vector4(float w) : w(w)
{
    // Allows creating a default Vector4 as wither a direction (w=0) or position (w=1).
}

Vector4::Vector4(const Vector3& other, float w) : Vector4(other.x, other.y, other.z, w)
{

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
    // Mult is faster than div, so (div + 4x mult) rather than (4x div).
    scalar = 1.0f / scalar;
    return Vector4(x * scalar, y * scalar, z * scalar, w * scalar);
}

Vector4& Vector4::operator/=(float scalar)
{
    // Mult is faster than div, so (div + 4x mult) rather than (4x div).
    scalar = 1.0f / scalar;
    x *= scalar;
    y *= scalar;
    z *= scalar;
    w *= scalar;
    return *this;
}

Vector4 Vector4::operator*(const Vector4& other) const
{
    return Vector4(x * other.x, y * other.y, z * other.z, w * other.w);
}

Vector4& Vector4::Normalize()
{
    // Get length squared. If zero, it means we can't normalize!
    float lengthSq = GetLengthSq();
    if(Math::IsZero(lengthSq))
    {
        return *this;
    }

    // To normalize, we divide each component by the length of the vector.
    // Or in other words, we can multiply by (1 / length).
    float oneOverLength = Math::InvSqrt(lengthSq);
    x *= oneOverLength;
    y *= oneOverLength;
    z *= oneOverLength;
    w *= oneOverLength;
    return *this;
}

/*static*/ Vector4 Vector4::Normalize(const Vector4& v)
{
    Vector4 v2 = v;
    v2.Normalize();
    return v2;
}

float Vector4::Dot(const Vector4& lhs, const Vector4& rhs)
{
    return (lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z + lhs.w * rhs.w);
}

Vector4 Vector4::Cross(const Vector4& lhs, const Vector4& rhs)
{
    // Cross product isn't really defined for Vector4.
    // What we really mean here is to treat the x/y/z part as two 3D vectors.

    // For the w component, we'll multiply the two values for the return.
    // So, 1*1=1, 0*1=0, 0*0=0.
    // I don't know at all whether this makes sense, but it seems reasonable at the moment ;D
    return Vector4(lhs.y * rhs.z - lhs.z * rhs.y,
                   lhs.z * rhs.x - lhs.x * rhs.z,
                   lhs.x * rhs.y - lhs.y * rhs.x,
                   lhs.w * rhs.w);
}

std::ostream& operator<<(std::ostream& os, const Vector4& v)
{
    os << "(" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << ")";
    return os;
}
