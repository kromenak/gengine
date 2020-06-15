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
	return FromRadians(quaternion.GetEulerAngles().y);
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
