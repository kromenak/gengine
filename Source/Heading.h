//
// Heading.h
//
// Clark Kromenaker
//
// A heading in GK3 is a yaw-only rotation (about the y-axis) that is defined in degrees from 0 to 360.
//
#pragma once

#include <string>

#include "GMath.h"
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
	static Heading FromDirection(const Vector3& direction);
	
	float ToDegrees() const { return mDegrees; }
	float ToRadians() const { return Math::ToRadians(mDegrees); }
	Quaternion ToQuaternion() const { return Quaternion(Vector3::UnitY, ToRadians()); }
	Vector3 ToVector() const { return ToQuaternion().Rotate(Vector3::UnitZ); }
	
	bool IsValid() const { return mDegrees >= 0.0f && mDegrees <= 360.0f; }
	
	std::string ToString() const { return std::to_string(mDegrees); }
	
private:
	Heading() { }
	
	// A heading is represented internally as degrees 0-360.
	float mDegrees = -1.0f;
	
	void SetDegrees(float degrees);
};

std::ostream& operator<<(std::ostream& os, const Heading& h);
