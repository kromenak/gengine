#include "Vector2.h"

#include "MathStringUtil.h"
#include "Vector3.h"

Vector2 Vector2::Zero(0.0f, 0.0f);
Vector2 Vector2::One(1.0f, 1.0f);
Vector2 Vector2::UnitX(1.0f, 0.0f);
Vector2 Vector2::UnitY(0.0f, 1.0f);

Vector2::Vector2(float x, float y) : x(x), y(y)
{

}

Vector2::Vector2(const Vector2& other) : x(other.x), y(other.y)
{

}

Vector2::Vector2(const Vector3& other) : x(other.x), y(other.y)
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
    // Mult is faster than div, so (div + 2x mult) rather than (2x div).
    scalar = 1 / scalar;
    return Vector2(x * scalar, y * scalar);
}

Vector2& Vector2::operator/=(float scalar)
{
    // Mult is faster than div, so (div + 2x mult) rather than (2x div).
    scalar = 1 / scalar;
    x *= scalar;
    y *= scalar;
    return *this;
}

Vector2 Vector2::operator*(const Vector2& other) const
{
    return Vector2(x * other.x, y * other.y);
}

Vector2& Vector2::Normalize()
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
    return *this;
}

/*static*/ Vector2 Vector2::Normalize(const Vector2& v)
{
    Vector2 v2 = v;
    v2.Normalize();
    return v2;
}

/*static*/ float Vector2::Dot(const Vector2& lhs, const Vector2& rhs)
{
    return (lhs.x * rhs.x + lhs.y * rhs.y);
}

/*static*/ Vector2 Vector2::Lerp(const Vector2& from, const Vector2& to, float t)
{
    return ((1.0f - t) * from) + (t * to);
}

/*static*/ Vector2 Vector2::Project(const Vector2& a, const Vector2& b)
{
    // See Vector3 class for explanation.
    return (b * (Dot(a, b) / Dot(b, b)));
}

/*static*/ Vector2 Vector2::Reject(const Vector2& a, const Vector2& b)
{
    // See Vector3 class for explanation.
    return (a - (b * (Dot(a, b) / Dot(b, b))));
}

/*static*/ Vector2 Vector2::Parse(const std::string& string, char delimiter)
{
    Vector2 vector;
    TryParse(string, vector, delimiter);
    return vector;
}

/*static*/ bool Vector2::TryParse(const std::string& string, Vector2& vector, char delimiter)
{
    // Get rid of all leading and trailing cruft - just numbers.
    std::string workingString = MathStringUtil::TrimToNumbersOnly(string);
    if(workingString.empty()) { return false; }

    // Find the comma index. If not present, this isn't the right form, so we fail.
    std::size_t commaIndex = workingString.find(delimiter);
    if(commaIndex == std::string::npos) { return false; }

    // Split at the comma.
    std::string firstNum = workingString.substr(0, commaIndex);
    std::string secondNum = workingString.substr(commaIndex + 1, std::string::npos);

    // Make sure the number strings appear valid.
    MathStringUtil::TrimToValidNumberString(firstNum);
    MathStringUtil::TrimToValidNumberString(secondNum);

    // Convert to numbers and return.
    vector.x = std::stof(firstNum);
    vector.y = std::stof(secondNum);
    return true;
}

std::string Vector2::ToString(char delimiter, char openChar, char closeChar) const
{
    // Create a string like "(5, -10)".
    std::string result;
    result.push_back(openChar);
    result += std::to_string(x);
    result.push_back(delimiter);
    result += std::to_string(y);
    result.push_back(closeChar);
    return result;
}

std::ostream& operator<<(std::ostream& os, const Vector2& v)
{
    os << "(" << v.x << ", " << v.y << ")";
    return os;
}
