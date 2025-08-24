#include "GKObject.h"

#include <sstream>

#include "MeshRenderer.h"
#include "PersistState.h"
#include "ReportManager.h"

TYPEINFO_INIT(GKObject, Actor, 31)
{
    TYPEINFO_VAR(GKObject, VariableType::String, mNoun);
    TYPEINFO_VAR(GKObject, VariableType::String, mVerb);
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

void GKObject::SetFixedLightingColor(const Color32& color)
{
    for(Material& material : GetMeshRenderer()->GetMaterials())
    {
        // Set ambient color to exact desired color.
        material.SetColor("uAmbientColor", color);

        // When using fixed lighting, global light color doesn't factor into lighting calculation.
        // Set to black so it's effectively ignored.
        material.SetColor("uLightColor", Color32::Black);
    }
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

void GKObject::OnPersist(PersistState& ps)
{
    // Every GKObject needs its position/rotation/scale saved. These can change during the scene.
    // I don't want to put OnPersist functions in non-GK3 classes, so that's why I get/set the transform values here.
    Vector3 position = GetTransform()->GetPosition();
    ps.Xfer(PERSIST_VAR(position));
    GetTransform()->SetPosition(position);

    Quaternion rotation = GetTransform()->GetRotation();
    ps.Xfer(PERSIST_VAR(rotation));
    GetTransform()->SetRotation(rotation);

    Vector3 scale = GetTransform()->GetScale();
    ps.Xfer(PERSIST_VAR(scale));
    GetTransform()->SetScale(scale);

    // Also, the active state is an important GKObject property.
    bool active = IsActive();
    ps.Xfer(PERSIST_VAR(active));
    SetActive(active);
}