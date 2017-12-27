//
// MeshComponent.cpp
//
// Clark Kromenaker
//
#include "MeshComponent.h"
#include "GLVertexArray.h"
#include "Model.h"
#include "Mesh.h"
#include "Services.h"

MeshComponent::MeshComponent(Actor* owner) : Component(owner)
{
    Services::GetRenderer()->AddMeshComponent(this);
}

MeshComponent::~MeshComponent()
{
    Services::GetRenderer()->RemoveMeshComponent(this);
}

void MeshComponent::Update(float deltaTime)
{
    Quaternion quat = mOwner->GetRotation();
    
    Vector3 axis;
    float angle;
    quat.GetAxisAngle(axis, angle);
    
    axis = Vector3::UnitY;
    angle += deltaTime;
    angle = Math::Mod(angle, Math::k2Pi);
    quat.Set(axis, angle);
    
    mOwner->SetRotation(quat);
}

void MeshComponent::Render()
{
    // Early out if nothing to render.
    if(mMeshes.size() == 0) { return; }
    
    // Update the world transform for the shader.
    Matrix4 worldTransform = mOwner->GetWorldTransformMatrix();
    Services::GetRenderer()->SetWorldTransformMatrix(worldTransform);
    
    // Draw meshes.
    //mMeshes[0]->GetVertexArray()->Draw();
    for(auto& mesh : mMeshes)
    {
        if(mesh != nullptr)
        {
            GLVertexArray* vertexArray = mesh->GetVertexArray();
            if(vertexArray != nullptr)
            {
                // Render the thing!
                vertexArray->Draw();
            }
        }
    }
}

void MeshComponent::SetMesh(Mesh* mesh)
{
    mMeshes.clear();
    mMeshes.push_back(mesh);
}

void MeshComponent::SetModel(Model* model)
{
    mMeshes.clear();
    for(auto& mesh : model->GetMeshes())
    {
        mMeshes.push_back(mesh);
    }
}
