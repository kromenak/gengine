//
// Vector3.h
//
// Clark Kromenaker
//
#include "Vector3.h"

Vector3 Vector3::Zero(0.0f, 0.0f, 0.0f);
Vector3 Vector3::One(1.0f, 1.0f, 1.0f);
Vector3 Vector3::UnitX(1.0f, 0.0f, 0.0f);
Vector3 Vector3::UnitY(0.0f, 1.0f, 0.0f);
Vector3 Vector3::UnitZ(0.0f, 0.0f, 1.0f);

Vector3::Vector3() : Vector3(0.0f, 0.0f, 0.0f)
{
    
}

Vector3::Vector3(float x, float y, float z) : x(x), y(y), z(z)
{
    
}

Vector3::Vector3(float x, float y) : x(x), y(y), z(0.0f)
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

Vector3& Vector3::Normalize()
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
    return *this;
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
    return Vector3(x / scalar, y / scalar, z / scalar);
}

Vector3& Vector3::operator/=(float scalar)
{
    x /= scalar;
    y /= scalar;
    z /= scalar;
    return *this;
}

float Vector3::Dot(Vector3 lhs, Vector3 rhs)
{
    return (lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z);
}

Vector3 Vector3::Cross(Vector3 lhs, Vector3 rhs)
{
    Vector3 result(lhs.y * rhs.z - lhs.z * rhs.y,
                   lhs.z * rhs.x - lhs.x * rhs.z,
                   lhs.x * rhs.y - lhs.y * rhs.x);
    return result;
}

Vector3 Vector3::Lerp(Vector3 from, Vector3 to, float t)
{
    return ((1.0f - t) * from) + (t * to);
}

std::ostream& operator<<(std::ostream& os, const Vector3& v)
{
    os << std::setprecision(9) << "(" << v.GetX() << ", " << v.GetY() << ", " << v.GetZ() << ")";
    return os;
}
