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

    // Any GKObject with a MeshRenderer should save some info about mesh actor position/rotation, mesh position/rotation, and submesh vertex positions.
    // For GKProps: often needed to remember mesh changes due to animations. Opening the desk drawer in Montreaux's Office, for example. Or a dirt prop after digging it with the shovel.
    // For GKActors: animations often leave the model actor in odd poses or positions. Grace climbing the wall at Montreaux's or Gabe spying in the Cemetery are two examples.
    MeshRenderer* meshRenderer = GetMeshRenderer();
    if(meshRenderer != nullptr)
    {
        // Particularly for GKActor, where the model actor is separate from the GKActor, saving/loading the model actor position/rotation/scale is important.
        // This technically isn't needed for GKProp, but no harm besides a bit of wasted space.
        Vector3 meshPosition = meshRenderer->GetOwner()->GetTransform()->GetPosition();
        ps.Xfer(PERSIST_VAR(meshPosition));
        meshRenderer->GetOwner()->GetTransform()->SetPosition(meshPosition);

        Quaternion meshRotation = meshRenderer->GetOwner()->GetTransform()->GetRotation();
        ps.Xfer(PERSIST_VAR(meshRotation));
        meshRenderer->GetOwner()->GetTransform()->SetRotation(meshRotation);

        Vector3 meshScale = meshRenderer->GetOwner()->GetTransform()->GetScale();
        ps.Xfer(PERSIST_VAR(meshScale));
        meshRenderer->GetOwner()->GetTransform()->SetScale(meshScale);

        //TODO: May want to serialize the 3D model and make sure it matches on load.

        // Iterate and save/load each mesh.
        //TODO: May want to have safeguards against mesh count or submesh count or vertex count changing here?
        for(Mesh* mesh : meshRenderer->GetMeshes())
        {
            // Save the mesh's matrix, which tracks any local movement due to animations.
            Matrix4& matrix = mesh->GetMeshToLocalMatrix();
            ps.Xfer(PERSIST_VAR(matrix));

            // Some animations may actually modify vertex positions, so we need to save/load those too.
            for(Submesh* submesh : mesh->GetSubmeshes())
            {
                for(int i = 0; i < submesh->GetVertexCount(); ++i)
                {
                    Vector3 vertexPosition = submesh->GetVertexPosition(i);
                    ps.Xfer(PERSIST_VAR(vertexPosition));
                    submesh->SetVertexPosition(i, vertexPosition);
                }
                submesh->SetPositions(submesh->GetPositions());
            }
        }
    }
}