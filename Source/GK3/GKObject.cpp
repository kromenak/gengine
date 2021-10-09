#include "GKObject.h"

#include <sstream>

#include "Services.h"

GKObject::GKObject() : Actor()
{
	
}

void GKObject::SetHeading(const Heading& heading)
{
	SetRotation(heading.ToQuaternion());
}

Heading GKObject::GetHeading() const
{
	return Heading::FromQuaternion(GetRotation());
}

void GKObject::DumpPosition()
{
    std::stringstream ss;
    ss << "actor '" << GetNoun() << "' ";
    ss << "h=" << GetHeading() << ", ";
    ss << "pos=" << GetPosition();
    Services::GetReports()->Log("Dump", ss.str());
}
