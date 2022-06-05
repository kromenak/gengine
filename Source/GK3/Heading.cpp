#include "Heading.h"

#include "Vector2.h"

/*static*/ Heading Heading::None = Heading();

/*static*/ Heading Heading::FromDegrees(float degrees)
{
	Heading heading;
	heading.SetDegrees(degrees);
	return heading;
}

/*static*/ Heading Heading::FromRadians(float radians)
{
	return FromDegrees(Math::ToDegrees(radians));
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

void Heading::SetDegrees(float degrees)
{
	mDegrees = degrees;
	
	// Make sure value is within range of -360 to 360.
	// Note this also works for negative values. (-361 % 360 = -1).
	mDegrees = Math::Mod(mDegrees, 360.0f);
	
	// If less than zero...just add 360!
	if(mDegrees < 0.0f)
	{
		mDegrees += 360.0f;
	}
}

std::ostream& operator<<(std::ostream& os, const Heading& h)
{
	os << h.ToString();
	return os;
}
