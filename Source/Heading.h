//
// Heading.h
//
// Clark Kromenaker
//
// A heading in GK3 is a yaw-only rotation (about the y-axis) that is usually
// defined in degrees from 0 to 360.
//
// A heading is often, along with a position, to define a placement for an actor.
//
#pragma once

#include "Math.h"
#include "Quaternion.h"
#include "Vector3.h"

class Heading
{
public:
	// Represents a "no heading" or "invalid heading" option.
	// Just internally leaves degrees at -1.
	static Heading None;
	
	static Heading FromDegrees(float degrees);
	static Heading FromRadians(float radians);
	static Heading FromQuaternion(const Quaternion& quaternion);
	
	float ToDegrees() const { return mDegrees; }
	float ToRadians() const { return Math::ToRadians(mDegrees); }
	Quaternion ToQuaternion() const { return Quaternion(Vector3::UnitY, ToRadians()); }
	Vector3 ToVector() const { return ToQuaternion().Rotate(Vector3::UnitZ); }
	
	bool IsValid() const { return mDegrees >= 0.0f && mDegrees <= 360.0f; }
	
private:
	Heading() { }
	
	float mDegrees = -1.0f;
	
	void SetDegrees(float degrees);
};
