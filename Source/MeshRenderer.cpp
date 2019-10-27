//
// MeshRenderer.cpp
//
// Clark Kromenaker
//
#include "MeshRenderer.h"

#include "Actor.h"
#include "Debug.h"
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

void MeshRenderer::RenderOpaque()
{
	Matrix4 actorWorldTransform = GetOwner()->GetTransform()->GetLocalToWorldMatrix();
	
	int materialIndex = 0;
	int maxMaterialIndex = static_cast<int>(mMaterials.size()) - 1;
	
	for(int i = 0; i < mMeshes.size(); i++)
	{
		Matrix4 meshWorldTransformMatrix = actorWorldTransform * mMeshes[i]->GetLocalTransformMatrix();
		
		auto submeshes = mMeshes[i]->GetSubmeshes();
		for(int j = 0; j < submeshes.size(); j++)
		{
			Material& material = mMaterials[materialIndex];
			
			// Ignore translucent rendering.
			if(!material.IsTranslucent())
			{
				// Activate material.
				material.SetWorldTransformMatrix(meshWorldTransformMatrix);
				material.Activate();
				
				// Render the submesh!
				submeshes[j]->Render();
			}
			
			// Draw debug axes if desired.
			if(Debug::RenderSubmeshLocalAxes())
			{
				Debug::DrawAxes(meshWorldTransformMatrix);
			}
			
			// Increase material index, but not above the max.
			materialIndex = Math::Min(materialIndex + 1, maxMaterialIndex);
		}
	}
	
	if(Debug::RenderSubmeshLocalAxes())
	{
		Debug::DrawAxes(actorWorldTransform);
	}
}

void MeshRenderer::RenderTranslucent()
{
	Matrix4 actorWorldTransform = GetOwner()->GetTransform()->GetLocalToWorldMatrix();
	
	int materialIndex = 0;
	int maxMaterialIndex = static_cast<int>(mMaterials.size()) - 1;
	
	for(int i = 0; i < mMeshes.size(); i++)
	{
		Matrix4 meshWorldTransform = actorWorldTransform * mMeshes[i]->GetLocalTransformMatrix();
		
		auto submeshes = mMeshes[i]->GetSubmeshes();
		for(int j = 0; j < submeshes.size(); j++)
		{
			Material& material = mMaterials[materialIndex];
			
			// Ignore opaque rendering.
			if(material.IsTranslucent())
			{
				// Activate material.
				material.SetWorldTransformMatrix(meshWorldTransform);
				material.Activate();
				
				// Render the submesh!
				submeshes[j]->Render();
			}
			
			// Increase material index, but not above the max.
			materialIndex = Math::Min(materialIndex + 1, maxMaterialIndex);
		}
	}
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
		if(!submesh->GetTextureName().empty())
		{
			Texture* tex = Services::GetAssets()->LoadTexture(submesh->GetTextureName());
			m.SetDiffuseTexture(tex);
		}
		
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

Material* MeshRenderer::GetMaterial(int index)
{
	if(index >= 0 && index < mMaterials.size())
	{
		return &mMaterials[index];
	}
	return nullptr;
}

Material* MeshRenderer::GetMaterial(int meshIndex, int submeshIndex)
{
	if(meshIndex >= 0 && meshIndex < mMeshes.size())
	{
		// Determine offset of materials for this mesh.
		int actualIndex = 0;
		for(int i = 0; i < meshIndex; i++)
		{
			actualIndex += mMeshes[i]->GetSubmeshCount();
		}
		actualIndex += submeshIndex;
		
		return GetMaterial(actualIndex);
	}
	return nullptr;
}

Mesh* MeshRenderer::GetMesh(int index) const
{
	if(index >= 0 && index < mMeshes.size())
	{
		return mMeshes[index];
	}
	return nullptr;
}

Matrix4 MeshRenderer::GetMeshWorldTransform(int index) const
{
	if(index >= 0 && index < mMeshes.size())
	{
		return GetOwner()->GetTransform()->GetLocalToWorldMatrix() * mMeshes[index]->GetLocalTransformMatrix();
	}
	return Matrix4::Identity;
}

bool MeshRenderer::Raycast(const Ray& ray)
{
	Matrix4 localToWorldMatrix = GetOwner()->GetComponent<Transform>()->GetLocalToWorldMatrix();
	//Matrix4 worldToLocalMatrix = mTransform->GetWorldToLocalMatrix();
	
	// Raycast against triangles in the mesh.
	for(auto& mesh : mMeshes)
	{
		Matrix4 meshWorldTransform = localToWorldMatrix * mesh->GetLocalTransformMatrix();
		Matrix4 worldToModelMatrix = meshWorldTransform.Inverse();
		
		Vector3 rayLocalPos = worldToModelMatrix.TransformPoint(ray.GetOrigin());
		Vector3 rayLocalDir = worldToModelMatrix.Transform(ray.GetDirection());
		rayLocalDir.Normalize();
		Ray localRay(rayLocalPos, rayLocalDir);
		
		if(mesh->Raycast(localRay))
		{
			return true;
		}
	}
	
	// Ray did not intersect with any part of the mesh renderer.
	return false;
}
