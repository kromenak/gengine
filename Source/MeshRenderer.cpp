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

TYPE_DEF_CHILD(Component, MeshRenderer);

MeshRenderer::MeshRenderer(Actor* owner) : Component(owner)
{
    Services::GetRenderer()->AddMeshRenderer(this);
}

MeshRenderer::~MeshRenderer()
{
    Services::GetRenderer()->RemoveMeshRenderer(this);
}

std::vector<RenderPacket> MeshRenderer::GetRenderPackets()
{
	Matrix4 actorWorldTransform = mOwner->GetTransform()->GetLocalToWorldMatrix();
	
	int materialIndex = 0;
	int maxMaterialIndex = static_cast<int>(mMaterials.size()) - 1;
	
	// For each mesh to be rendered, create a render packet.
    std::vector<RenderPacket> packets;
    for(int i = 0; i < mMeshes.size(); i++)
    {
        RenderPacket packet;
        packet.mesh = mMeshes[i];
        
        // Each mesh has one or more submeshes. Each submesh can/should have a different material.
		// Each packet is given the offset into materials for the first submesh, plus the number of materials to use.
		// In cases where there aren't enough materials for number of submeshes, the last material is used by default.
		// If NO materials are present, the render packet will handle the null case.
		if(mMaterials.size() > 0)
		{
			packet.materials = &mMaterials[materialIndex];
			packet.materialsCount = Math::Min(mMeshes[i]->GetSubmeshCount(), (int)mMaterials.size() - materialIndex);
			materialIndex = Math::Min(materialIndex + mMeshes[i]->GetSubmeshCount(), maxMaterialIndex);
		}
		
        // Save world transform for this packet.
        packet.worldTransform = actorWorldTransform * mMeshes[i]->GetLocalTransformMatrix();
		
		// Add packet to packets array.
        packets.push_back(packet);
    }
    return packets;
}

void MeshRenderer::SetModel(Model* model)
{
    if(model == nullptr) { return; }
	mModel = model;
    
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
	// Add mesh to array.
	mMeshes.push_back(mesh);
	
	// Create a material for each submesh.
	const std::vector<Submesh*>& submeshes = mesh->GetSubmeshes();
	for(auto& submesh : submeshes)
	{
		// Generate materials for each mesh.
		Material m;
		
		// Load and set texture reference.
		Texture* tex = Services::GetAssets()->LoadTexture(submesh->GetTextureName());
		m.SetDiffuseTexture(tex);
		
		// Add to materials list.
		mMaterials.push_back(m);
	}
}

void MeshRenderer::SetMaterial(int index, Material material)
{
	if(index >= 0 && index < mMaterials.size())
	{
		mMaterials[index] = material;
	}
}
