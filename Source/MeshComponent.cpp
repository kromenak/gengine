//
// MeshComponent.cpp
//
// Clark Kromenaker
//
#include "MeshComponent.h"
#include "GLVertexArray.h"
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

void MeshComponent::Render()
{
    if(mMesh != nullptr)
    {
        GLVertexArray* vertexArray = mMesh->GetVertexArray();
        if(vertexArray != nullptr)
        {
            // Update the world transform for the shader.
            Matrix4 worldTransform = mOwner->GetWorldTransformMatrix();
            Services::GetRenderer()->SetWorldTransformMatrix(worldTransform);
            
            // Render the thing!
            vertexArray->Draw();
        }
    }
}
