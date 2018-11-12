//
// MeshRenderer.cpp
//
// Clark Kromenaker
//
#include "MeshRenderer.h"

#include "Actor.h"
#include "Mesh.h"
#include "Model.h"
#include "Services.h"
#include "Texture.h"

extern Mesh* axes;

TYPE_DEF_CHILD(Component, MeshRenderer);

MeshRenderer::MeshRenderer(Actor* owner) : Component(owner)
{
    Services::GetRenderer()->AddMeshRenderer(this);
}

MeshRenderer::~MeshRenderer()
{
    Services::GetRenderer()->RemoveMeshRenderer(this);
}

/*
void MeshRenderer::Render()
{
    Matrix4 worldTransform = mOwner->GetWorldTransformMatrix();
    
    // Draws a little axes indicator at the position of the actor.
    Services::GetRenderer()->SetWorldTransformMatrix(worldTransform);
    glBindTexture(GL_TEXTURE_2D, 0);
    axes->Render();
    
    // Early out if nothing to render.
    if(mMeshes.size() == 0) { return; }
    
    // Render all the meshes!
    for(int i = 0; i < mMeshes.size(); i++)
    {
        // Can't render without any materials.
        if(mMaterials.size() == 0) { continue; }
        
        // Can't render if this mesh is null.
        if(mMeshes[i] == nullptr) { continue; }
        
        // Usually there's a one-to-one mapping of mesh to material.
        // But if not for some reason, use the last material available.
        int materialIndex = Math::Clamp(i, 0, (int)mMaterials.size() - 1);
        mMaterials[materialIndex].Activate();
        
        // Each mesh has it's own local offset and rotation.
        // So, we'll combine that into the world transform matrix before rendering.
        Matrix4 finalMatrix = worldTransform * mMeshes[i]->GetLocalTransformMatrix();
        mMaterials[materialIndex].SetWorldTransformMatrix(finalMatrix);
		
		// Finally, we render the mesh.
        mMeshes[i]->Render();
        
        // This bit draws local axes for each mesh, for debugging position/rotation.
        // TODO: Would be cool to turn this on/off as needed.
        glBindTexture(GL_TEXTURE_2D, 0);
        axes->Render();
    }
}
*/

std::vector<RenderPacket> MeshRenderer::GetRenderPackets()
{
    std::vector<RenderPacket> packets;
    for(int i = 0; i < mMeshes.size(); i++)
    {
        RenderPacket packet;
        packet.mesh = mMeshes[i];
        
        // Usually there's a one-to-one mapping of mesh to material.
        // But if not for some reason, use the max.
		// TODO: If zero materials, use a default "error" material.
        int materialIndex = Math::Clamp(i, 0, (int)mMaterials.size() - 1);
        packet.material = mMaterials[materialIndex];
        
        // Save world transform for this packet.
		Matrix4 worldTransform = mOwner->GetTransform()->GetLocalToWorldMatrix();
        Matrix4 finalMatrix = worldTransform * mMeshes[i]->GetLocalTransformMatrix();
        packet.worldTransform = finalMatrix;
        
        packets.push_back(packet);
    }
    return packets;
}

void MeshRenderer::SetModel(Model* model)
{
    if(model == nullptr) { return; }
    
    // Clear any existing.
    mMeshes.clear();
    mMaterials.clear();
    
    // Add each mesh.
    for(auto& mesh : model->GetMeshes())
    {
        AddMesh(mesh);
    }
}

void MeshRenderer::SetMesh(Mesh* mesh)
{
    mMeshes.clear();
    mMaterials.clear();
    AddMesh(mesh);
}

void MeshRenderer::AddMesh(Mesh* mesh)
{
    mMeshes.push_back(mesh);
	
    // Generate materials for each mesh.
    Material m;
    
    // Load and set texture reference.
    Texture* tex = Services::GetAssets()->LoadTexture(mesh->GetTextureName());
    m.SetDiffuseTexture(tex);
    
    // Add to materials list.
    mMaterials.push_back(m);
}

void MeshRenderer::SetMaterial(int index, Material material)
{
	if(index >= 0 && index < mMaterials.size())
	{
		mMaterials[index] = material;
	}
}
