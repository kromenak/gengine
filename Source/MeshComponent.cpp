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

extern GLVertexArray* axes;

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
    //mOwner->Rotate(Vector3::UnitY, deltaTime);
}

void MeshComponent::Render()
{
    // Update the world transform for the shader.
    Matrix4 worldTransform = mOwner->GetWorldTransformMatrix();
    
    // Draws a little axes indicator at the position of the actor.
    Services::GetRenderer()->SetWorldTransformMatrix(worldTransform);
    glBindTexture(GL_TEXTURE_2D, 0);
    axes->DrawLines();
    
    // Early out if nothing to render.
    if(mMeshes.size() == 0) { return; }
    
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
            // Each mesh has it's own local offset and rotation.
            // So, we'll combine that into the world transform matrix before rendering.
            Matrix4 transMatrix = Matrix4::MakeTranslate(mMeshes[i]->GetOffset());
            Matrix4 rotMatrix = Matrix4::MakeRotate(mMeshes[i]->GetRotation());
            Matrix4 finalMatrix = worldTransform * transMatrix * rotMatrix;
            Services::GetRenderer()->SetWorldTransformMatrix(finalMatrix);
            
            mMeshes[i]->Render();
            
            // This bit draws local axes for the model, for debugging.
            // TODO: Would be cool to turn this on/off as needed.
            glBindTexture(GL_TEXTURE_2D, 0);
            axes->DrawLines();
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
    if(model == nullptr)
    {
        std::cout << "Model is null!" << std::endl;
        return;
    }
    
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
