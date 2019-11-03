//
// Quaternion.cpp
//
// Clark Kromenaker
//
#include "Quaternion.h"

#include "Matrix3.h"
#include "Vector3.h"

Quaternion Quaternion::Zero(0.0f, 0.0f, 0.0f, 0.0f);
Quaternion Quaternion::Identity(0.0f, 0.0f, 0.0f, 1.0f);

Quaternion::Quaternion() :
    x(0.0f), y(0.0f), z(0.0f), w(1.0f)
{
    
}

Quaternion::Quaternion(float x, float y, float z, float w) :
    x(x), y(y), z(z), w(w)
{
    
}

Quaternion::Quaternion(const Vector3& axis, float angle)
{
    Set(axis, angle);
}

Quaternion::Quaternion(const Vector3& vector) :
    x(vector.GetX()),
    y(vector.GetY()),
    z(vector.GetZ()),
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

bool Quaternion::IsZero() const
{
    return Math::IsZero(x*x + y*y + z*z + w*w);
}

bool Quaternion::IsUnit() const
{
    return Math::IsZero(1.0f - (x*x + y*y + z*z + w*w));
}

bool Quaternion::IsIdentity() const
{
    return (Math::IsZero(x) &&
            Math::IsZero(y) &&
            Math::IsZero(z) &&
            Math::IsZero(1.0f - w));
}

void Quaternion::Set(const Vector3& axis, float angle)
{
    // Special case: axis is the zero vector. Just set to identity.
    float lengthSq = axis.GetLengthSq();
    if(Math::IsZero(lengthSq))
    {
        MakeIdentity();
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
    x = axis.GetX() * sinAndNormalize;
    y = axis.GetY() * sinAndNormalize;
    z = axis.GetZ() * sinAndNormalize;
}

// Creates a quaternion representing a rotation from one vector to another.
// Method is based on an article by minorlogic from GameDev.net, and
// referenced in Essential Mathematics for Games.
void Quaternion::Set(const Vector3& from, const Vector3& to)
{
    // Take cross product between two vectors to get axis of rotation.
    Vector3 axis = Vector3::Cross(from, to);
    
    // Set axis values and set w to dot product of two vectors.
    Set(axis.GetX(), axis.GetY(), axis.GetZ(), Vector3::Dot(from, to));
   
    // Quaternion is now ||from||*||to||*(cos(theta), r * sin(theta)).
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
        if(from.GetZ() * from.GetZ() > from.GetX() * from.GetX())
        {
            Set(0.0f, from.GetZ(), -from.GetY(), 0.0f);
        }
        else
        {
            Set(from.GetY(), -from.GetX(), 0.0f, 0.0f);
        }
    }
    
    // now when we normalize, we'll be dividing by sqrt(2*(1+cos(theta))), which is
    // what we want for r*sin(theta) to give us r*sin(theta/2)  (see pages 487-488)
    //
    // w will become
    //                 1+cos(theta)
    //            ----------------------
    //            sqrt(2*(1+cos(theta)))
    // which simplifies to
    //                cos(theta/2)
    Normalize();
}

void Quaternion::Set(const Matrix3 &rotation)
{
    float trace = rotation.GetTrace();
    if(trace > 0.0f)
    {
        float s = Math::Sqrt(trace + 1.0f);
        w = s * 0.5f;
        
        float recip = 0.5f / s;
        x = (rotation(2, 1) - rotation(1, 2)) * recip;
        y = (rotation(0, 2) - rotation(2, 0)) * recip;
        z = (rotation(1, 0) - rotation(0, 1)) * recip;
    }
    else
    {
        unsigned int i = 0;
        if(rotation(1, 1) > rotation(0, 0))
        {
            i = 1;
        }
        if(rotation(2, 2) > rotation(i, i))
        {
            i = 2;
        }
        unsigned int j = (i + 1) % 3;
        unsigned int k = (j + 1) % 3;
        float s = Math::Sqrt(rotation(i, i) - rotation(j, j) - rotation(k, k) + 1.0f);
        (*this)[i] = 0.5f * s;
        
        float recip = 0.5f / s;
        w = (rotation(k, j) - rotation(j, k)) * recip;
        (*this)[j] = (rotation(j, i) + rotation(i, j)) * recip;
        (*this)[k] = (rotation(k, i) + rotation(i, k)) * recip;
    }
}

void Quaternion::Set(float xRadians, float yRadians, float zRadians)
{
    // Since the internal representation of an angle in a quaternion is
    // half the angle, we cut each passed in angle in half.
    xRadians *= 0.5f;
    yRadians *= 0.5f;
    zRadians *= 0.5f;
    
    // To convert fixed angles to a quaternion, we basically need to
    // create three quaternions to represent rotation around each axis
    // and concatenate them to get a final quaternion.
    
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

void Quaternion::Set(Vector3 forward, Vector3 up, Vector3 right)
{
    /*
     Vector3 forward = Vector3.Normalize(forward);
     Vector3 right = Vector3.Normalize(Vector3.Cross(up, forward));
     Vector3 up = Vector3.Cross(forward, right);
     var m00 = right.x;
     var m01 = right.y;
     var m02 = right.z;
     var m10 = up.x;
     var m11 = up.y;
     var m12 = up.z;
     var m20 = forward.x;
     var m21 = forward.y;
     var m22 = forward.z;
    */
    
    /*
    float m00 = right.GetX();
    float m01 = right.GetY();
    float m02 = right.GetZ();
    float m10 = up.GetX();
    float m11 = up.GetY();
    float m12 = up.GetZ();
    float m20 = forward.GetX();
    float m21 = forward.GetY();
    float m22 = forward.GetZ();
    */
}

void Quaternion::GetAxisAngle(Vector3& axis, float& angle) const
{
    angle = 2.0f * acosf(w);
    
    float length = Math::Sqrt(1.0f - (w * w));
    if(Math::IsZero(length))
    {
        axis.SetX(0);
        axis.SetY(0);
        axis.SetZ(0);
    }
    else
    {
        length = 1.0f / length;
        axis.SetX(x * length);
        axis.SetY(y * length);
        axis.SetZ(z * length);
    }
}

Vector3 Quaternion::GetEulerAngles() const
{
	// Taken from Wikipedia (https://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles)
	Vector3 result;
	
	// X-axis
	float sinr_cosp = 2 * (w * x + y * z);
	float cosr_cosp = 1 - (2 * (x * x + y * y));
	result.SetX(Math::Atan2(sinr_cosp, cosr_cosp));
	
	// Y-axis
	float sinp = 2 * (w * y - z * x);
	if(Math::Abs(sinp) >= 1)
	{
		result.SetY(Math::MagnitudeSign(Math::kPi / 2, sinp));
	}
	else
	{
		result.SetY(Math::Asin(sinp));
	}
	
	// Z-axis
	float siny_cosp = 2 * (w * z + x * y);
	float cosy_cosp = 1 - (2 * (y * y + z * z));
	result.SetZ(Math::Atan2(siny_cosp, cosy_cosp));
	
	return result;
}

float Quaternion::GetLength() const
{
    return Math::Sqrt(x*x + y*y + z*z + w*w);
}

void Quaternion::Normalize()
{
    float lengthSq = (x * x) + (y * y) + (z * z) + (w * w);
    if(Math::IsZero(lengthSq))
    {
        MakeZero();
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

Quaternion Quaternion::Inverse(const Quaternion& quat)
{
    // Calculate length squared, so we know if the quaternion is zero or not.
    float lengthSq = (quat.x * quat.x) + (quat.y * quat.y) + (quat.z * quat.z) + (quat.w * quat.w);
    
    // If length is zero, we just return the identity matrix.
    // This is technically an invalid operation, btw.
    if(Math::IsZero(lengthSq))
    {
        return Identity;
    }
    
    // We basically just want to negate our axis. The angle (w) doesn't negate.
    // As mentioned in EMFG, a fully negated quat rotates to same angle but other way around axis.
    // The inverse just rotates the same angle around opposite axis.
    // Also, the equation can be simpler if quat is unit vector...
    float recip = 1.0f / lengthSq;
    return Quaternion(-recip * quat.x, -recip * quat.y, -recip * quat.z, recip * quat.w);
}

const Quaternion& Quaternion::Invert()
{
    // Very similar to above, but affects an instance of a quat, instead of returning a new one.
    float lengthSq = (x * x) + (y * y) + (z * z) + (w * w);
    if(Math::IsZero(lengthSq))
    {
        return *this;
    }
    
    float recip = 1.0f / lengthSq;
    w *= recip;
    x *= -recip;
    y *= -recip;
    z *= -recip;
    return *this;
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

Quaternion& Quaternion::operator*=(const Quaternion &other)
{
    Set(w * other.x + x * other.w + y * other.z - z * other.y,
        w * other.y + y * other.w + z * other.x - x * other.z,
        w * other.z + z * other.w + x * other.y - y * other.x,
        w * other.w - x * other.x - y * other.y - z * other.z);
    return *this;
}

float Quaternion::Dot(const Quaternion& quat1, const Quaternion& quat2)
{
    return (quat1.x * quat2.x + quat1.y * quat2.y + quat1.z * quat2.z + quat1.w * quat2.w);
}

Vector3 Quaternion::Rotate(const Vector3& vector) const
{
    //ASSERT( IsUnit() );
    
    float vMult = 2.0f * (x * vector.GetX() + y * vector.GetY() + z * vector.GetZ());
    float crossMult = 2.0f * w;
    float pMult = crossMult * w - 1.0f;
    
    return Vector3(pMult * vector.GetX() + vMult * x + crossMult * (y * vector.GetZ() - z * vector.GetY()),
                   pMult * vector.GetY() + vMult * y + crossMult * (z * vector.GetX() - x * vector.GetZ()),
                   pMult * vector.GetZ() + vMult * z + crossMult * (x * vector.GetY() - y * vector.GetX()));
}

void Quaternion::Lerp(Quaternion &result, const Quaternion &start, const Quaternion &end, float t)
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

void Quaternion::Slerp(Quaternion &result, const Quaternion &start, const Quaternion &end, float t)
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

std::ostream& operator<<(std::ostream& os, const Quaternion& q)
{
    Vector3 axis;
    float angle;
    q.GetAxisAngle(axis, angle);
    
    os << axis << ", " << Math::ToDegrees(angle);
    return os;
}
