//
// Heading.cpp
//
// Clark Kromenaker
//
#include "Heading.h"

Heading Heading::None = Heading();

Heading Heading::FromDegrees(float degrees)
{
	Heading heading;
	heading.SetDegrees(degrees);
	return heading;
}

Heading Heading::FromRadians(float radians)
{
	return FromDegrees(Math::ToDegrees(radians));
}

Heading Heading::FromQuaternion(const Quaternion& quaternion)
{
    // A heading is really only meaningful for rotations about the up axis (Y in our case).
    // We can't assume the passed in quaternion is always ONLY about the y-axis, so we need to isolate just the Y part.
    
    // We can do this by zeroing out x/z and normalize - this works b/c we're isolating one the three main coordinate axes.
    // Math is described here: https://stackoverflow.com/questions/5782658/extracting-yaw-from-a-quaternion
    Quaternion yRotation = quaternion;
    yRotation.x = 0.0f;
    yRotation.z = 0.0f;
    yRotation.Normalize();
	return FromRadians(yRotation.GetAngle());
}

Heading Heading::FromDirection(const Vector3& direction)
{
	// Zero out Y-component and renormalize.
	Vector3 dir = direction;
	dir.y = 0.0f;
	dir.Normalize();
	
	// Calculate axis and angle of rotation.
	Vector3 axis = Vector3::Cross(Vector3::UnitZ, dir).Normalize();
	float angle = Math::Acos(Vector3::Dot(Vector3::UnitZ, dir));
	
	// Create heading from quaternion.
	return Heading::FromQuaternion(Quaternion(axis, angle));
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
