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
	return FromRadians(quaternion.GetEulerAngles().GetY());
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
