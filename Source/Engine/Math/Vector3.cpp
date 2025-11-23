#include "Vector3.h"

#include <iomanip> // std::setprecision

#include "MathStringUtil.h"
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

/*static*/ Vector3 Vector3::Parse(const std::string& string, char delimiter)
{
    Vector3 vector;
    TryParse(string, vector, delimiter);
    return vector;
}

/*static*/ bool Vector3::TryParse(const std::string& string, Vector3& vector, char delimiter)
{
    // Get rid of all leading and trailing cruft - just numbers.
    std::string workingString = MathStringUtil::TrimToNumbersOnly(string);
    if(workingString.empty()) { return false; }

    // Find the two commas.
    std::size_t firstCommaIndex = workingString.find(delimiter);
    if(firstCommaIndex == std::string::npos) { return false; }
    std::size_t secondCommaIndex = workingString.find(delimiter, firstCommaIndex + 1);
    if(secondCommaIndex == std::string::npos) { return false; }

    // Split at commas.
    std::string firstNum = workingString.substr(0, firstCommaIndex);
    std::string secondNum = workingString.substr(firstCommaIndex + 1, secondCommaIndex - firstCommaIndex - 1);
    std::string thirdNum = workingString.substr(secondCommaIndex + 1, std::string::npos);

    // Make sure the number strings appear valid.
    MathStringUtil::TrimToValidNumberString(firstNum);
    MathStringUtil::TrimToValidNumberString(secondNum);
    MathStringUtil::TrimToValidNumberString(thirdNum);

    // Convert to numbers and return.
    vector.x = std::stof(firstNum);
    vector.y = std::stof(secondNum);
    vector.z = std::stof(thirdNum);
    return true;
}

std::string Vector3::ToString(char delimiter, char openChar, char closeChar) const
{
    // Create a string like "(5, -10, 20)".
    std::string result;
    result.push_back(openChar);
    result += std::to_string(x);
    result.push_back(delimiter);
    result += std::to_string(y);
    result.push_back(delimiter);
    result += std::to_string(z);
    result.push_back(closeChar);
    return result;
}

std::ostream& operator<<(std::ostream& os, const Vector3& v)
{
    os << std::setprecision(9) << "(" << v.x << ", " << v.y << ", " << v.z << ")";
    return os;
}
