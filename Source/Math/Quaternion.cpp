#include "Quaternion.h"

#include "Matrix3.h"
#include "Vector3.h"

Quaternion Quaternion::Zero(0.0f, 0.0f, 0.0f, 0.0f);
Quaternion Quaternion::Identity(0.0f, 0.0f, 0.0f, 1.0f);

Quaternion::Quaternion(float x, float y, float z, float w) :
    x(x), y(y), z(z), w(w)
{

}

Quaternion::Quaternion(const Vector3& axis, float angle)
{
    Set(axis, angle);
}

Quaternion::Quaternion(const Vector3& vector) :
    x(vector.x),
    y(vector.y),
    z(vector.z),
    w(0.0f)
{

}

Quaternion::Quaternion(const Matrix3& rotation)
{
    Set(rotation);
}

Quaternion::Quaternion(const Quaternion& other)
{
    x = other.x;
    y = other.y;
    z = other.z;
    w = other.w;
}

Quaternion& Quaternion::operator=(const Quaternion& other)
{
    x = other.x;
    y = other.y;
    z = other.z;
    w = other.w;
    return *this;
}

bool Quaternion::operator==(const Quaternion& other) const
{
    return (Math::AreEqual(x, other.x) &&
            Math::AreEqual(y, other.y) &&
            Math::AreEqual(z, other.z) &&
            Math::AreEqual(w, other.w));
}

bool Quaternion::operator!=(const Quaternion& other) const
{
    return !(Math::AreEqual(x, other.x) &&
             Math::AreEqual(y, other.y) &&
             Math::AreEqual(z, other.z) &&
             Math::AreEqual(w, other.w));
}

Quaternion Quaternion::operator+(const Quaternion& other) const
{
    return Quaternion(x + other.x, y + other.y, z + other.z, w + other.w);
}

Quaternion& Quaternion::operator+=(const Quaternion& other)
{
    x += other.x;
    y += other.y;
    z += other.z;
    w += other.w;
    return *this;
}

Quaternion Quaternion::operator-(const Quaternion& other) const
{
    return Quaternion(x - other.x, y - other.y, z - other.z, w - other.w);
}

Quaternion& Quaternion::operator-=(const Quaternion& other)
{
    x -= other.x;
    y -= other.y;
    z -= other.z;
    w -= other.w;
    return *this;
}

Quaternion Quaternion::operator-() const
{
    return Quaternion(-x, -y, -z, -w);
}

Quaternion operator*(float scalar, const Quaternion& quat)
{
    return Quaternion(scalar * quat.x, scalar * quat.y, scalar * quat.z, scalar * quat.w);
}

Quaternion& Quaternion::operator*=(float scalar)
{
    x *= scalar;
    y *= scalar;
    z *= scalar;
    w *= scalar;
    return *this;
}

Quaternion Quaternion::operator*(const Quaternion& other) const
{
    return Quaternion(w * other.x + x * other.w + y * other.z - z * other.y,
                      w * other.y + y * other.w + z * other.x - x * other.z,
                      w * other.z + z * other.w + x * other.y - y * other.x,
                      w * other.w - x * other.x - y * other.y - z * other.z);
}

Quaternion& Quaternion::operator*=(const Quaternion& other)
{
    // Important to not overwrite x/y/z/w right away b/c they are used for subsequent calculations!
    float newX = w * other.x + x * other.w + y * other.z - z * other.y;
    float newY = w * other.y + y * other.w + z * other.x - x * other.z;
    float newZ = w * other.z + z * other.w + x * other.y - y * other.x;
    float newW = w * other.w - x * other.x - y * other.y - z * other.z;
    x = newX;
    y = newY;
    z = newZ;
    w = newW;
    return *this;
}

Vector3 Quaternion::Rotate(const Vector3& vector) const
{
    //ASSERT(IsUnit());
    float vMult = 2.0f * (x * vector.x + y * vector.y + z * vector.z);
    float crossMult = 2.0f * w;
    float pMult = crossMult * w - 1.0f;

    return Vector3(pMult * vector.x + vMult * x + crossMult * (y * vector.z - z * vector.y),
                   pMult * vector.y + vMult * y + crossMult * (z * vector.x - x * vector.z),
                   pMult * vector.z + vMult * z + crossMult * (x * vector.y - y * vector.x));
}

void Quaternion::Set(const Vector3& axis, float angle)
{
    // Special case: axis is the zero vector. Just set to identity.
    float lengthSq = axis.GetLengthSq();
    if(Math::IsZero(lengthSq))
    {
        x = y = z = 0.0f;
        w = 1.0f;
        return;
    }

    // We need the half angle for our calculations.
    angle *= 0.5f;

    // The w component is easy - just cosine of the half angle.
    w = Math::Cos(angle);

    // For the axis, the equation is v = sin(angle) * (v/||v||).
    // So, this combines the "sin(angle) / ||v|| part into one value.
    // We can then just multiply by the axis components.
    float sinAndNormalize = Math::Sin(angle) / Math::Sqrt(lengthSq);
    x = axis.x * sinAndNormalize;
    y = axis.y * sinAndNormalize;
    z = axis.z * sinAndNormalize;
}

float Quaternion::GetAngle() const
{
    return 2.0f * Math::Acos(w);
}

void Quaternion::GetAxisAngle(Vector3& axis, float& angle) const
{
    // scalar part = cos(θ/2)
    // So, we can extract the angle directly.
    angle = 2.0f * Math::Acos(w);

    // vector part = axis * sin(θ/2)
    // In other words, the vector part is the axis, but with length of sin(θ/2).
    // We assume quaternion is unit length, so subtracting w^2 gives us length of just vector part (aka sin(θ/2)).
    float length = Math::Sqrt(1.0f - (w * w));

    // Normalize vector part to get the axis!
    if(Math::IsZero(length))
    {
        axis = Vector3::Zero;
    }
    else
    {
        length = 1.0f / length;
        axis.x = x * length;
        axis.y = y * length;
        axis.z = z * length;
    }
}

void Quaternion::Set(const Vector3& from, const Vector3& to)
{
    // Creates quaternion that is a rotation between two direction vectors.r.
    // Method based on an article by minorlogic from GameDev.net, referenced in Essential Mathematics book.

    // Take cross product between two vectors to get axis of rotation.
    Vector3 axis = Vector3::Cross(from, to);

    // Set axis values and set w to dot product of two vectors.
    x = axis.x;
    y = axis.y;
    z = axis.z;
    w = Vector3::Dot(from, to);

    // Quaternion is now ||from|| * ||to|| * (cos(theta), r * sin(theta)).
    // Normalize to remove the ||from||*||to|| part.
    Normalize();

    // Quaternion is now (cos(theta), r * sin(theta)).
    // We want it to be (cos(theta/2), r * sin(theta/2)).
    w += 1.0f;

    // Before normalizing, handle case where from/to vectors are opposing.
    // If result of dot product was -1 (and then we added 1), then the vectors
    // were facing opposite directions.
    if(w <= Math::kEpsilon)
    {
        // Rotate pi radians around the orthogonal vector.
        if(from.z * from.z > from.x * from.x)
        {
            x = 0.0f;
            y = from.z;
            z = -from.y;
        }
        else
        {
            x = from.y;
            y = -from.x;
            z = 0.0f;
        }
        w = 0.0f;
    }

    // now when we normalize, we'll be dividing by sqrt(2*(1+cos(theta))), which is
    // what we want for r*sin(theta) to give us r*sin(theta/2) (see pages 487-488)
    //
    // w will become
    //                 1+cos(theta)
    //            ----------------------
    //            sqrt(2*(1+cos(theta)))
    // which simplifies to
    //                cos(theta/2)
    Normalize();
}

void Quaternion::Set(const Matrix3& rotation)
{
    // The method used to convert to a quaternion depends on whether the trace is positive or not.
    float trace = rotation.GetTrace();
    if(trace >= 0.0f)
    {
        // Given an axis and angle, q = (sin(θ/2) * axis, cos(θ/2)).

        // From a rotation matrix, we can *almost* get this data!
        // Vector part: from the matrix, calculate (R21 - R12, R02 - R20, R10 - R01). This equals (2 * sin(θ) * axis).
        // Scalar part: trace of matrix equals (2 * cos(θ) + 1).
        // So, from a rotation matrix, we can get q2 = (2 * sin(θ) * axis, 2 * cos(θ) + 1).

        // To convert q2 to q1 format, we can simply add one to the scalar part and normalize!
        // See (EMFG, 191)

        // Calculate vector part (2 * sin(θ) * axis).
        x = rotation(2, 1) - rotation(1, 2);
        y = rotation(0, 2) - rotation(2, 0);
        z = rotation(1, 0) - rotation(0, 1);

        // Calculate scalar part (2 * cos(θ) + 1).
        // Add one to get "+ 2" instead, which is required for normalization to give correct result.
        w = trace + 1.0f;

        // Normalize to force quaternion into correct format.
        Normalize();
    }
    else
    {
        // When the trace is less than zero, the above method doesn't work (the vector part calculation returns zero).
        // This alternative method works in that case. (EMFG, 191)

        // Determine which diagonal value is largest.
        unsigned int i = 0;
        if(rotation(1, 1) > rotation(0, 0))
        {
            i = 1;
        }
        if(rotation(2, 2) > rotation(i, i))
        {
            i = 2;
        }

        // Depending on what i ended up being, determine j & k with wraparound.
        // If i = 0, j = 1 & k = 2.
        // If i = 1, j = 2 & k = 0 (and so on).
        unsigned int j = (i + 1) % 3;
        unsigned int k = (j + 1) % 3;

        // The first term can be calculated by subtracting the three diagonal values in the right order and adding 1.
        // The *0.5f bit is a sneaky way to normalize the result.
        float s = Math::Sqrt(rotation(i, i) - rotation(j, j) - rotation(k, k) + 1.0f);
        (*this)[i] = 0.5f * s;

        // Calculate remaining terms, with similar sneaky normalization techniques.
        float recip = 0.5f / s;
        w = (rotation(k, j) - rotation(j, k)) * recip;
        (*this)[j] = (rotation(j, i) + rotation(i, j)) * recip;
        (*this)[k] = (rotation(k, i) + rotation(i, k)) * recip;
    }
}

void Quaternion::Set(float xRadians, float yRadians, float zRadians)
{
    // To convert fixed angles to a quaternion, we basically need to
    // create three quaternions to represent rotation around each axis
    // and concatenate them to get a final quaternion.

    // Since the internal representation of an angle in a quaternion is
    // half the angle, we cut each passed in angle in half.
    xRadians *= 0.5f;
    yRadians *= 0.5f;
    zRadians *= 0.5f;

    // This method does that, but without creating the quaternions,
    // and just doing the math. (EMFG, 193)
    float sinX = Math::Sin(xRadians);
    float cosX = Math::Cos(xRadians);

    float sinY = Math::Sin(yRadians);
    float cosY = Math::Cos(yRadians);

    float sinZ = Math::Sin(zRadians);
    float cosZ = Math::Cos(zRadians);

    w = cosX * cosY * cosZ - sinX * sinY * sinZ;
    x = sinX * cosY * cosZ + cosX * sinY * sinZ;
    y = cosX * sinY * cosZ - sinX * cosY * sinZ;
    z = cosX * cosY * sinZ + sinZ * sinY * cosX;
}

Vector3 Quaternion::GetEulerAngles() const
{
    // Taken from Wikipedia (https://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles)
    Vector3 result;

    // X-axis
    float sinr_cosp = 2 * (w * x + y * z);
    float cosr_cosp = 1 - (2 * (x * x + y * y));
    result.x = Math::Atan2(sinr_cosp, cosr_cosp);

    // Y-axis
    float sinp = 2 * (w * y - z * x);
    if(Math::Abs(sinp) >= 1)
    {
        result.y = Math::MagnitudeSign(Math::kPi / 2, sinp);
    }
    else
    {
        result.y = Math::Asin(sinp);
    }

    // Z-axis
    float siny_cosp = 2 * (w * z + x * y);
    float cosy_cosp = 1 - (2 * (y * y + z * z));
    result.z = Math::Atan2(siny_cosp, cosy_cosp);

    return result;
}

bool Quaternion::IsUnit() const
{
    return Math::IsZero(1.0f - (x * x + y * y + z * z + w * w));
}

float Quaternion::GetLength() const
{
    return Math::Sqrt(x * x + y * y + z * z + w * w);
}

void Quaternion::Normalize()
{
    float lengthSq = (x * x) + (y * y) + (z * z) + (w * w);
    if(Math::IsZero(lengthSq))
    {
        x = y = z = w = 0.0f;
    }
    else
    {
        float factor = Math::InvSqrt(lengthSq);
        x *= factor;
        y *= factor;
        z *= factor;
        w *= factor;
    }
}

/*static*/ float Quaternion::Dot(const Quaternion& quat1, const Quaternion& quat2)
{
    return (quat1.x * quat2.x + quat1.y * quat2.y + quat1.z * quat2.z + quat1.w * quat2.w);
}

void Quaternion::Invert()
{
    // For unit length quaternions, the inverse is equal to the conjugate (just negate vector part).
    if(IsUnit())
    {
        x *= -1.0f;
        y *= -1.0f;
        z *= -1.0f;
    }
    else
    {
        // Calculate length squared, so we know if the quaternion is zero or not.
        float lengthSq = (x * x) + (y * y) + (z * z) + (w * w);

        // If length is zero, we just return the quaternion, unaffected.
        // This is technically an invalid operation, btw.
        if(Math::IsZero(lengthSq)) { return; }

        // For non-unit quaternions, inverse is calculated by dividing by quaternion (q) times conjugate (q*).
        // After doing the math/simplifying, this means we can just divide by length squared and negate the vector part.
        float recip = 1.0f / lengthSq;
        x *= -recip;
        y *= -recip;
        z *= -recip;
        w *= recip;
    }
}

/*static*/ Quaternion Quaternion::Inverse(const Quaternion& quat)
{
    Quaternion toInvert = quat;
    toInvert.Invert();
    return toInvert;
}

/*static*/ Quaternion Quaternion::Diff(const Quaternion& q2, const Quaternion& q1)
{
    // The idea behind "diff" is similar to how scalar and vector subtraction work.
    // You can do scale "s2 - s1" (calculate value you'd add to s1 to get s2).
    // You can do vector "v2 - v1" (calculate value you'd add to v1 to get v2).
    // Quaternion subtraction doesn't do the same thing, but a similar value can be calculated using inverses!

    // Inverse of q1 represents opposite rotation of q1.
    // Then, multiplying by q2 leaves us with q, which you multiply by q1 to get q2.
    Quaternion diff = Inverse(q1) * q2;
    return diff;
}

/*static*/ void Quaternion::Lerp(Quaternion &result, const Quaternion &start, const Quaternion &end, float t)
{
    // Get cos of angle between quaternions.
    float cosTheta = Quaternion::Dot(start, end);

    // Set result to end by default.
    result = t * end;

    // If angle between quaternions is less than 90 degrees, use standard interpolation.
    // Otherwise, take the other path.
    if(cosTheta >= Math::kEpsilon)
    {
        result += (1.0f - t) * start;
    }
    else
    {
        result += (t - 1.0f) * start;
    }
}

/*static*/ void Quaternion::Slerp(Quaternion &result, const Quaternion &start, const Quaternion &end, float t)
{
    // Get cos of angle between quaternions.
    float cosTheta = Quaternion::Dot(start, end);
    float startInterp, endInterp;

    // If angle between quaternions is less than 90 degrees...
    if(cosTheta >= Math::kEpsilon)
    {
        // If angle is greater than zero, use standard slerp.
        if((1.0f - cosTheta) > Math::kEpsilon)
        {
            float theta = Math::Acos(cosTheta);
            float recipSinTheta = 1.0f / Math::Sin(theta);

            startInterp = Math::Sin((1.0f - t) * theta) * recipSinTheta;
            endInterp = Math::Sin(t * theta) * recipSinTheta;
        }
        // Angle is close to zero, so use linear interpolation.
        else
        {
            startInterp = 1.0f - t;
            endInterp = t;
        }
    }
    // Otherwise, take the shorter route.
    else
    {
        // If angle is less than 180 degrees...
        if((1.0f + cosTheta) > Math::kEpsilon)
        {
            // ...use slerp w/ negation of start quaternion.
            float theta = Math::Acos(-cosTheta);
            float recipSinTheta = 1.0f / Math::Sin(theta);

            startInterp = Math::Sin((t - 1.0f) * theta) * recipSinTheta;
            endInterp = Math::Sin(t * theta) * recipSinTheta;
        }
        // Or angle is close to 180 degrees, so use lerp w/ negated start quat.
        else
        {
            startInterp = t - 1.0f;
            endInterp = t;
        }
    }
    result = startInterp * start + endInterp * end;
}

void Quaternion::Decompose(const Vector3& axis, Quaternion& aboutAxis) const
{
    // Use this version of Decompose if you don't care about the perpendicular axis - that's the (potentially) expensive part anyway.
    Vector3 proj = Vector3::Project(Vector3(x, y, z), axis);
    aboutAxis.x = proj.x;
    aboutAxis.y = proj.y;
    aboutAxis.z = proj.z;
    aboutAxis.w = w;
    aboutAxis.Normalize();
}

void Quaternion::Decompose(const Vector3& axis, Quaternion& aboutAxis, Quaternion& aboutPerpendicularAxis) const
{
    // This algorithm is referred to as "swing twist decomposition."
    // It allows you to decompose a quaterion into two parts:
    // 1) The "twist" - the part that rotates about a given axis/direction.
    // 2) The "swing" - the part that rotates about an axis perpendicular to the given axis/direction.

    // Project our axis onto desired axis.
    Vector3 proj = Vector3::Project(Vector3(x, y, z), axis);

    // Use that to generate quat with rotation about that axis.
    // This is the "twist."
    aboutAxis.x = proj.x;
    aboutAxis.y = proj.y;
    aboutAxis.z = proj.z;
    aboutAxis.w = w;
    aboutAxis.Normalize();

    // Use "twist" with original rotation to calculate the "swing."
    aboutPerpendicularAxis = Quaternion::Inverse(aboutAxis) * (*this);
}

Quaternion Quaternion::Isolate(const Vector3& axis) const
{
    Quaternion aboutAxis;
    Decompose(axis, aboutAxis);
    return aboutAxis;
}

Quaternion Quaternion::Discard(const Vector3& axis) const
{
    Quaternion aboutAxis;
    Quaternion aboutPerpAxis;
    Decompose(axis, aboutAxis, aboutPerpAxis);
    return aboutPerpAxis;
}

void Quaternion::IsolateY()
{
    // We can isolate just the rotation about the Y axis by zeroing x/z and normalizing.
    x = 0.0f;
    z = 0.0f;
    Normalize();

    // But watch out! This can result in a Zero quat if y/w were close to zero.
    // Revert to identity in that case.
    if(Math::IsZero(x) && Math::IsZero(y) && Math::IsZero(z) && Math::IsZero(w))
    {
        w = 1.0f;
    }
}

std::ostream& operator<<(std::ostream& os, const Quaternion& q)
{
    Vector3 axis;
    float angle;
    q.GetAxisAngle(axis, angle);

    os << axis << ", " << Math::ToDegrees(angle);
    return os;
}
