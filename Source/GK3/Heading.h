//
// Clark Kromenaker
//
// A heading is a yaw-only rotation (about the y-axis).
//
#pragma once

#include <iostream>
#include <string>

#include "GMath.h"
#include "Quaternion.h"
#include "Vector3.h"

class Heading
{
public:
	// Represents a "no heading" or "invalid heading" option.
	static Heading None;

    // Headings are currently implemented as immutable objects.
    // So, you can't modify a heading - you can only create a new one.
    static Heading FromRadians(float radians);
    static Heading FromDegrees(float degrees);
    static Heading FromDirection(const Vector3& direction);
	static Heading FromQuaternion(const Quaternion& quaternion);
    
	float ToDegrees() const { return Math::ToDegrees(mRadians); }
	float ToRadians() const { return mRadians; }
	Quaternion ToQuaternion() const { return Quaternion(Vector3::UnitY, mRadians); }
    Vector3 ToDirection() const { return Vector3::Normalize(Vector3(Math::Sin(mRadians), 0.0f, Math::Cos(mRadians)));  }

	bool IsValid() const { return mRadians >= 0.0f && mRadians <= Math::k2Pi; }
	
	std::string ToString() const { return std::to_string(ToDegrees()); }
	
private:
    // A heading is represented as an angle 0 to 2PI.
    // Default of -1 represents an invalid/unset heading.
    float mRadians = -1.0f;

    Heading() = default;
    void SetRadians(float radians);
};

std::ostream& operator<<(std::ostream& os, const Heading& h);
