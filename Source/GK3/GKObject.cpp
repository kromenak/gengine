#include "GKObject.h"

#include <sstream>

#include "MeshRenderer.h"
#include "ReportManager.h"

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
    gReportManager.Log("Dump", ss.str());
}

Vector3 GKObject::GetAudioPosition() const
{
    // Models in GK3 are often authored such that the "visual" position of the model does not match the world position.
    // It's usually more accurate to find the center-point of the mesh's AABB.
    MeshRenderer* meshRenderer = GetMeshRenderer();
    if(meshRenderer != nullptr)
    {
        return meshRenderer->GetAABB().GetCenter();
    }
    else
    {
        return GetWorldPosition();
    }
}