#include "Billboard.h"

#include "GameCamera.h"
#include "MeshRenderer.h"
#include "SceneManager.h"

Billboard::Billboard(Actor* owner) : Component(owner)
{
    mMeshRenderer = GetOwner()->GetComponent<MeshRenderer>();
    mInitialPos = GetRotatePosition();
}

void Billboard::OnUpdate(float deltaTime)
{
    // Get the camera's forward vector, using UnitZ as the default worst case.
    Vector3 camForwardDir = Vector3::UnitZ;
    Scene* scene = gSceneManager.GetScene();
    if(scene != nullptr)
    {
        GameCamera* camera = scene->GetCamera();
        if(camera != nullptr)
        {
            camForwardDir = camera->GetForward();
        }
    }
    
    // Use heading helpers to convert from facing direction to quaternion (ignoring Y axis).
    // This creates a rotation that points in the direction of the camera's forward vector.
    Quaternion rotation = Heading::FromDirection(camForwardDir).ToQuaternion();

    // Set this Actor's rotation to the generated rotation. This creates the billboard effect.
    // However, this *probably* also moves the Actor, since the visuals are usually not centered on the local pivot point - that's a problem!
    GetOwner()->SetRotation(rotation);

    // To counter the problem with rotating moving the actor, we need to calculate how much rotating the object also moved it and counter that movement.
    // Calculate how much the object moved from its initial position, and move it back by that amount.
    Vector3 currentToStart = mInitialPos - GetRotatePosition();
    GetOwner()->GetTransform()->Translate(currentToStart);
}

Vector3 Billboard::GetRotatePosition()
{
    const std::vector<Mesh*>& meshes = mMeshRenderer->GetMeshes();
    if(meshes.empty())
    {
        return Vector3::Zero;
    }
    else if(meshes.size() == 1)
    {
        // When there's a single mesh, that means we have one billboard to deal with.
        // Ideally, we'd like to do this, but it doesn't work in every case (see Chateau de Serres exterior for example):
        //return (GetOwner()->GetTransform()->GetLocalToWorldMatrix() * meshes[0]->GetMeshToLocalMatrix()).GetTranslation();

        // What DOES seem to work in all cases: calculate the bottom-center point of the billboard quad.
        Vector3 posSum;
        float minY = FLT_MAX;
        for(Submesh* submesh : meshes[0]->GetSubmeshes())
        {
            for(uint32_t i = 0; i < submesh->GetVertexCount(); ++i)
            {
                Vector3 pos = submesh->GetVertexPosition(i);
                posSum += pos;

                if(pos.y < minY)
                {
                    minY = pos.y;
                }
            }
            posSum /= static_cast<float>(submesh->GetVertexCount());
        }
        posSum.y = minY;

        // Transform that point to world space and return it.
        return (GetOwner()->GetTransform()->GetLocalToWorldMatrix() * meshes[0]->GetMeshToLocalMatrix()).TransformPoint(posSum);
    }
    else // more than one mesh (pine trees mainly)
    {
        // A bit tricky - ideally, you'd like a clear convention (e.g. always use mesh 0).
        // But in practice, billboard models are not consistent (see the trees in RC1 as an example).

        // The best I can do here is try to guess which mesh should be used via some hueristic.
        // We'll find which mesh's local space origin is closes to the AABB's center point.
        // This seems to work in all cases I could find.
        Vector3 center = mMeshRenderer->GetAABB().GetCenter();

        Vector3 bestMeshPoint;
        float smallestDistSq = FLT_MAX;
        for(Mesh* mesh : meshes)
        {
            Vector3 meshPoint = (GetOwner()->GetTransform()->GetLocalToWorldMatrix() * mesh->GetMeshToLocalMatrix()).GetTranslation();
            float distSq = (center - meshPoint).GetLengthSq();
            if(distSq < smallestDistSq)
            {
                bestMeshPoint = meshPoint;
                smallestDistSq = distSq;
            }
        }
        return bestMeshPoint;
    }
}