#include "MeshRenderer.h"

#include <cassert>

#include "Actor.h"
#include "Collisions.h"
#include "Debug.h"
#include "Model.h"
#include "Ray.h"
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

void MeshRenderer::Render(bool opaque, bool translucent)
{
    // Don't render if actor is inactive or component is disabled.
    if(!IsActiveAndEnabled()) { return; }

    // As we iterate & render each submesh, keep track of which submesh we are currently on.
    // This acts as an index into materials & visibility arrays.
    int submeshIndex = 0;

    // There can potentially be more submeshes than materials defined.
    // If so, any additional submeshes just use the last material.
    int maxMaterialIndex = static_cast<int>(mMaterials.size()) - 1;
    
    // Iterate meshes and render each in turn.
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    Matrix4 localToWorldMatrix = GetOwner()->GetTransform()->GetLocalToWorldMatrix();
    for(int i = 0; i < mMeshes.size(); i++)
    {
        // Mesh vertices are in "mesh space". Create matrix to convert to world space.
        Matrix4 meshToWorldMatrix = localToWorldMatrix * mMeshes[i]->GetMeshToLocalMatrix();
        
        // Iterate each submesh.
        auto submeshes = mMeshes[i]->GetSubmeshes();
        for(int j = 0; j < submeshes.size(); j++)
        {
            // Some meshes can have quite a few submeshes, but it seems wasteful to store visible bits for ALL of them.
            // So, we'll assume if some mesh has a ton of submeshes, that only the first 64 can be invisible. Everything >64 is always visible.
            bool submeshVisible = submeshIndex >= kMaxSubmeshes || !mSubmeshInvisible[submeshIndex];

            // Don't render anything if this submesh is invisible!
            if(submeshVisible)
            {
                int materialIndex = Math::Min(submeshIndex, maxMaterialIndex);
                Material& material = mMaterials[materialIndex];

                // If this is translucent, only render if want to render translucent stuff.
                // Or if this is opaque, only render if want to render opaque.
                if((opaque && !material.IsTranslucent()) ||
                   (translucent && material.IsTranslucent()))
                {
                    // Activate material.
                    material.Activate(meshToWorldMatrix);

                    // Render the submesh!
                    submeshes[j]->Render();

                    // Draw debug axes if desired.
                    if(Debug::RenderSubmeshLocalAxes())
                    {
                        Debug::DrawAxes(meshToWorldMatrix);
                    }

                    /*
                    // Uncomment to visualize normals.
                    int vcount = submeshes[j]->GetVertexCount();
                    for(int k = 0; k < vcount; ++k)
                    {
                        Matrix4 worldToMeshMatrix = Matrix4::Inverse(meshToWorldMatrix);
                        Vector3 lightPos = worldToMeshMatrix.TransformPoint(GEngine::Instance()->GetScene()->GetSceneData()->GetGlobalLightPosition());
                        Vector3 lightDir = Vector3::Normalize(lightPos - submeshes[j]->GetVertexPosition(k));
                        float dot = Vector3::Dot(submeshes[j]->GetVertexNormal(k), lightDir);
                        Color32 color(static_cast<int>(dot * 255), 0, 0);

                        Vector3 pos = submeshes[j]->GetVertexPosition(k);
                        pos = meshToWorldMatrix.TransformPoint(pos);

                        Vector3 normal = submeshes[j]->GetVertexNormal(k);
                        normal = meshToWorldMatrix.TransformNormal(normal);

                        Debug::DrawLine(pos, pos + normal, color);
                        //Debug::DrawLine(pos, pos + lightDir, Color32::Yellow);
                    }
                    */
                }
            }

            // Increase submesh index.
            ++submeshIndex;
        }
    }
    //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    
    // Optionally some AABB drawing.
    if(Debug::RenderAABBs())
    {
        DebugDrawAABBs();
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

std::string MeshRenderer::GetModelName() const
{
    return mModel != nullptr ? mModel->GetNameNoExtension() : std::string();
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
        Material m(mShader);
		
		// Load and set texture reference.
		if(!submesh->GetTextureName().empty())
		{
			Texture* tex = Services::GetAssets()->LoadSceneTexture(submesh->GetTextureName());
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
		return GetMaterial(GetIndexFromMeshSubmeshIndexes(meshIndex, submeshIndex));
	}
	return nullptr;
}

void MeshRenderer::SetVisibility(int meshIndex, int submeshIndex, bool visible)
{
    int index = GetIndexFromMeshSubmeshIndexes(meshIndex, submeshIndex);
    if(index < mSubmeshInvisible.size())
    {
        mSubmeshInvisible[index] = !visible;
    }
}

Mesh* MeshRenderer::GetMesh(int index) const
{
	if(index >= 0 && index < mMeshes.size())
	{
		return mMeshes[index];
	}
	return nullptr;
}

bool MeshRenderer::Raycast(const Ray& ray, RaycastHit& hitInfo)
{
	Matrix4 localToWorldMatrix = GetOwner()->GetTransform()->GetLocalToWorldMatrix();
	
	// Raycast against triangles in the mesh.
	for(auto& mesh : mMeshes)
	{
		// Calculate world->local space transform by creating object->local and inverting.
		Matrix4 meshToWorldMatrix = localToWorldMatrix * mesh->GetMeshToLocalMatrix();
        Matrix4 worldToMeshMatrix = Matrix4::InverseTransform(meshToWorldMatrix);
		
		// Transform the ray to object space.
        Vector3 rayLocalPos = worldToMeshMatrix.TransformPoint(ray.origin);
        Vector3 rayLocalDir = worldToMeshMatrix.TransformVector(ray.direction);
		rayLocalDir.Normalize();
		Ray localRay(rayLocalPos, rayLocalDir);
		
		// See if the local ray intersects the local space triangles of the mesh.
		if(mesh->Raycast(localRay, hitInfo))
		{
            // The "t" value calculated in Raycast is in mesh space, but caller probably needs it in world space.
            // So, convert "t" back to world space before returning.
            Vector3 hitPoint = localRay.GetPoint(hitInfo.t);
            Vector3 hitPointWorldPos = meshToWorldMatrix.TransformPoint(hitPoint);
            //Debug::DrawSphere(Sphere(hitPointWorldPos, 1.0f), Color32::Red, 1.0f);
            hitInfo.t = (ray.origin - hitPointWorldPos).GetLength();
			return true;
		}
	}
	
	// Ray did not intersect with any part of the mesh renderer.
	return false;
}

AABB MeshRenderer::GetAABB() const
{
    if(mMeshes.empty()) { return AABB(); }

    // Calculate AABB that contains all meshes in the mesh renderer.
    AABB toReturn;
    for(int i = 0; i < mMeshes.size(); ++i)
    {
        Matrix4 meshToWorldMatrix = GetOwner()->GetTransform()->GetLocalToWorldMatrix() * mMeshes[i]->GetMeshToLocalMatrix();

        const AABB& meshAABB = mMeshes[i]->GetAABB();
        Vector3 worldMin = meshToWorldMatrix.TransformPoint(meshAABB.GetMin());
        Vector3 worldMax = meshToWorldMatrix.TransformPoint(meshAABB.GetMax());

        if(i == 0)
        {
            toReturn = AABB(worldMin, worldMax);
        }
        else
        {
            toReturn.GrowToContain(worldMin);
            toReturn.GrowToContain(worldMax);
        }
    }
    return toReturn;
}

void MeshRenderer::DebugDrawAABBs(const Color32& color, const Color32& meshColor)
{
    // The local-to-world matrix for this Actor is required for all meshes.
	Matrix4 localToWorldMatrix = GetOwner()->GetTransform()->GetLocalToWorldMatrix();
	
	// Raycast against triangles in the mesh.
	for(auto& mesh : mMeshes)
	{
		// Calculate mesh->world matrix.
		Matrix4 meshToWorldMatrix = localToWorldMatrix * mesh->GetMeshToLocalMatrix();
	
		// Debug draw the AABB.
        Debug::DrawAABB(mesh->GetAABB(), meshColor, 0.0f, &meshToWorldMatrix);
	}

    // Also draw the MeshRenderer's OVERALL AABB in a different color.
    Debug::DrawAABB(GetAABB(), color);
}

int MeshRenderer::GetIndexFromMeshSubmeshIndexes(int meshIndex, int submeshIndex)
{
    // Some submesh data is stored in a 1-dimensional array (e.g. materials, visibility)
    // But we usually have a meshIndex/submeshIndex combo. We need to convert that to a 1-D index.
    int actualIndex = 0;
    for(int i = 0; i < meshIndex; ++i)
    {
        actualIndex += mMeshes[i]->GetSubmeshCount();
    }
    actualIndex += submeshIndex;
    return actualIndex;
}
