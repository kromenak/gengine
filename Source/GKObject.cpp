//
// GKObject.cpp
//
// Clark Kromenaker
//
#include "GKObject.h"

GKObject::GKObject() : Actor()
{
	
}

void GKObject::SetHeading(const Heading& heading)
{
	SetRotation(Quaternion(Vector3::UnitY, heading.ToRadians()));
}

Heading GKObject::GetHeading() const
{
	return Heading::FromQuaternion(GetRotation());
}
