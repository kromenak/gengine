#include "SceneConstruction.h"

#include "Actor.h"
#include "Debug.h"
#include "Mesh.h"
#include "MeshRenderer.h"
#include "Model.h"
#include "Scene.h"
#include "SceneData.h"
#include "Services.h"
#include "WalkerBoundary.h"

extern Mesh* quad;

void SceneConstruction::Init(Scene* scene, SceneData* sceneData)
{
    mScene = scene;
    mSceneData = sceneData;

    // If a camera bounds model exists for this scene, pass it along to the camera.
    for(auto& modelName : sceneData->GetCameraBoundsModelNames())
    {
        Model* model = Services::GetAssets()->LoadModel(modelName);
        if(model != nullptr)
        {
            mCameraBoundsModels.push_back(model);
        }
    }

    // For debugging - render walker bounds overlay on game world.
    WalkerBoundary* walkerBoundary = sceneData->GetWalkerBoundary();
    if(walkerBoundary != nullptr)
    {
        Actor* walkerBoundaryActor = new Actor();
        mWalkerBoundaryActor = walkerBoundaryActor;

        MeshRenderer* walkerBoundaryMeshRenderer = walkerBoundaryActor->AddComponent<MeshRenderer>();
        walkerBoundaryMeshRenderer->SetMesh(quad);

        Material m;
        m.SetDiffuseTexture(walkerBoundary->GetTexture());
        walkerBoundaryMeshRenderer->SetMaterial(0, m);

        Vector3 size = walkerBoundary->GetSize();
        Vector3 offset = walkerBoundary->GetOffset();
        offset.x = -offset.x + size.x * 0.5f;
        offset.z = -offset.y + size.y * 0.5f;
        offset.y = 0.1f; // Offset slightly up to avoid z-fighting with floor (in most scenes).

        walkerBoundaryActor->SetPosition(offset);
        walkerBoundaryActor->SetRotation(Quaternion(Vector3::UnitX, Math::kPiOver2));
        walkerBoundaryActor->SetScale(size);

        // Disable by default.
        walkerBoundaryActor->SetActive(false);
    }
}

void SceneConstruction::Render()
{
    // Draw camera bounds if desired.
    if(mShowCameraBounds)
    {
        for(Model* cameraBoundsModel : mCameraBoundsModels)
        {
            for(Mesh* mesh : cameraBoundsModel->GetMeshes())
            {
                // Camera bounds meshes are assumed to have a world position of (0, 0, 0). So their "local to world " is the identity.
                // As a result, drawing at "mesh to local" directly works out.
                Debug::DrawAABB(mesh->GetAABB(), Color32::Red, 0.0f, &mesh->GetMeshToLocalMatrix());
            }
        }
    }

    // Render unwalkable rects when the walker boundary visualization is active.
    if(mWalkerBoundaryActor != nullptr && mWalkerBoundaryActor->IsActive())
    {
        mSceneData->GetWalkerBoundary()->DrawUnwalkableRects();
    }
}

void SceneConstruction::SetShowWalkerBoundary(bool show)
{
    if(mWalkerBoundaryActor != nullptr)
    {
        mWalkerBoundaryActor->SetActive(show);
    }
}

bool SceneConstruction::GetShowWalkerBoundary() const
{
    return mWalkerBoundaryActor != nullptr ? mWalkerBoundaryActor->IsActive() : false;
}