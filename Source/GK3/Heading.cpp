#include "Heading.h"

#include "Vector2.h"

/*static*/ Heading Heading::None = Heading();

/*static*/ Heading Heading::FromRadians(float radians)
{
    Heading heading;
    heading.SetRadians(radians);
    return heading;
}

/*static*/ Heading Heading::FromDegrees(float degrees)
{
    return FromRadians(Math::ToRadians(degrees));
}

/*static*/ Heading Heading::FromDirection(const Vector3& direction)
{
    // Convert to 2D vector on x/z plane and renormalize.
    Vector2 dir(direction.x, direction.z);
    dir.Normalize();

    // Calculate angle of rotation. This angle is just on the x/z plane, so we can use atan2.
    // In our case, "y" axis is to the right, "x" axis is up, so we PURPOSELY pass args as x/y rather than y/x.
    float angle = Math::Atan2(dir.x, dir.y);

    // Create heading from angle.
    return Heading::FromRadians(angle);
}

/*static*/ Heading Heading::FromQuaternion(const Quaternion& quaternion)
{
    // A heading is really only meaningful for rotations about the up axis (Y in our case).
    // We can't assume the passed in quaternion is always ONLY about the y-axis, so we need to isolate just the Y part.
    Quaternion yRotation = quaternion;
    yRotation.IsolateY();

    // Converting the quaternion directly to an angle (using GetAxisAngle) gives some undesirable results when wrapping around from 360 to 0.
    // To avoid this, just convert the quaternion to a direction, and then pass off to that logic to calculate final heading.
    return FromDirection(yRotation.Rotate(Vector3::UnitZ));
}

void Heading::SetRadians(float radians)
{
    mRadians = radians;

    // Make sure stored value is within valid range (0 - 2*PI).
    // Note this also works for negative values.
    mRadians = Math::Mod(mRadians, Math::k2Pi);

    // If less than zero...just add 2PI!
    if(mRadians < 0.0f)
    {
        mRadians += Math::k2Pi;
    }
}

std::ostream& operator<<(std::ostream& os, const Heading& h)
{
	os << h.ToString();
	return os;
}
