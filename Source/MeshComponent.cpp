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
#include "Texture.h"

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
    
    // Render the things!
    for(int i = 0; i < mMeshes.size(); i++)
    {
        // See if there's a texture at the same index as this mesh.
        if(mTextures.size() > i && mTextures[i] != nullptr)
        {
            mTextures[i]->Activate();
        }
        
        // Really render it now!
        if(mMeshes[i] != nullptr)
        {
            mMeshes[i]->Render();
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
    // Populate meshes array.
    mMeshes.clear();
    for(auto& mesh : model->GetMeshes())
    {
        mMeshes.push_back(mesh);
    }
    
    // Retrieve all textures associated with the model.
    mTextures.clear();
    for(auto& textureName : model->GetTextureNames())
    {
        // If empty, insert a null for no texture and keep indexes aligned.
        if(textureName.empty())
        {
            mTextures.push_back(nullptr);
            continue;
        }
        
        // Texture names don't include the extension, so we need to add them.
        //TODO: Should maybe do this in the asset manager?
        textureName.append(".BMP");
        
        // Load texture into memory. We don't really care here whether it is
        // null or not. If it is null, it'll just look incorrect at runtime.
        Texture* tex = Services::GetAssets()->LoadTexture(textureName);
        if(tex == nullptr)
        {
            std::cout << "Encountered null texture in MeshComponent" << std::endl;
        }
        mTextures.push_back(tex);
    }
}

void MeshComponent::AddTexture(Texture* texture)
{
    mTextures.push_back(texture);
}
