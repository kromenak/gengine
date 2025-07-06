#include "Vector3.h"

#include <iomanip> // std::setprecision

#include "Vector2.h"

Vector3 Vector3::Zero(0.0f, 0.0f, 0.0f);
Vector3 Vector3::One(1.0f, 1.0f, 1.0f);
Vector3 Vector3::UnitX(1.0f, 0.0f, 0.0f);
Vector3 Vector3::UnitY(0.0f, 1.0f, 0.0f);
Vector3 Vector3::UnitZ(0.0f, 0.0f, 1.0f);

Vector3::Vector3(float x, float y, float z) : x(x), y(y), z(z)
{

}

Vector3::Vector3(float x, float y) : x(x), y(y), z(0.0f)
{

}

Vector3::Vector3(const Vector2& other) : x(other.x), y(other.y), z(0.0f)
{

}

Vector3::Vector3(const Vector3& other) : x(other.x), y(other.y), z(other.z)
{

}

Vector3& Vector3::operator=(const Vector3& other)
{
    x = other.x;
    y = other.y;
    z = other.z;
    return *this;
}

bool Vector3::operator==(const Vector3& other) const
{
    return (Math::AreEqual(x, other.x) &&
            Math::AreEqual(y, other.y) &&
            Math::AreEqual(z, other.z));
}

bool Vector3::operator!=(const Vector3& other) const
{
    return !(Math::AreEqual(x, other.x) &&
             Math::AreEqual(y, other.y) &&
             Math::AreEqual(z, other.z));
}

Vector3 Vector3::operator+(const Vector3 &other) const
{
    return Vector3(x + other.x, y + other.y, z + other.z);
}

Vector3& Vector3::operator+=(const Vector3 &other)
{
    x += other.x;
    y += other.y;
    z += other.z;
    return *this;
}

Vector3 Vector3::operator-(const Vector3 &other) const
{
    return Vector3(x - other.x, y - other.y, z - other.z);
}

Vector3& Vector3::operator-=(const Vector3 &other)
{
    x -= other.x;
    y -= other.y;
    z -= other.z;
    return *this;
}

Vector3 Vector3::operator-() const
{
    return Vector3(-x, -y, -z);
}

Vector3 Vector3::operator*(float scalar) const
{
    return Vector3(x * scalar, y * scalar, z * scalar);
}

Vector3 operator*(float scalar, const Vector3& vector)
{
    return Vector3(vector.x * scalar, vector.y * scalar, vector.z * scalar);
}

Vector3& Vector3::operator*=(float scalar)
{
    x *= scalar;
    y *= scalar;
    z *= scalar;
    return *this;
}

Vector3 Vector3::operator/(float scalar) const
{
    // Mult is faster than div, so (div + 3x mult) rather than (3x div).
    scalar = 1.0f / scalar;
    return Vector3(x * scalar, y * scalar, z * scalar);
}

Vector3& Vector3::operator/=(float scalar)
{
    // Mult is faster than div, so (div + 3x mult) rather than (3x div).
    scalar = 1.0f / scalar;
    x *= scalar;
    y *= scalar;
    z *= scalar;
    return *this;
}

Vector3 Vector3::operator*(const Vector3& other) const
{
    return Vector3(x * other.x, y * other.y, z * other.z);
}

Vector3& Vector3::Normalize()
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
    return *this;
}

/*static*/ Vector3 Vector3::Normalize(const Vector3& v)
{
    Vector3 v2 = v;
    v2.Normalize();
    return v2;
}

/*static*/ float Vector3::Dot(const Vector3& lhs, const Vector3& rhs)
{
    return (lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z);
}

/*static*/ Vector3 Vector3::Cross(const Vector3& lhs, const Vector3& rhs)
{
    return Vector3(lhs.y * rhs.z - lhs.z * rhs.y,
                   lhs.z * rhs.x - lhs.x * rhs.z,
                   lhs.x * rhs.y - lhs.y * rhs.x);
}

/*static*/ Vector3 Vector3::Lerp(const Vector3& from, const Vector3& to, float t)
{
    return ((1.0f - t) * from) + (t * to);
}

/*static*/ Vector3 Vector3::Project(const Vector3& a, const Vector3& b)
{
    // Calculates projection of vector a onto vector b. Requires that b is unit length.

    // This math actually does scalar projection ((a dot b) / ||b||).
    // And then it multiplies by (b / ||b||) to get a vector rather than a scalar.
    // Fortunately, we can avoid sqrt because b is normalized twice!
    return (b * (Dot(a, b) / Dot(b, b)));
}

/*static*/ Vector3 Vector3::Reject(const Vector3& a, const Vector3& b)
{
    // Basically calculate projection of a onto b (as above), then subtract from a.
    return (a - (b * (Dot(a, b) / Dot(b, b))));
}

std::ostream& operator<<(std::ostream& os, const Vector3& v)
{
    os << std::setprecision(9) << "(" << v.x << ", " << v.y << ", " << v.z << ")";
    return os;
}
