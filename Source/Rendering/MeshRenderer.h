//
// Clark Kromenaker
//
// Component for rendering meshes.
//
#pragma once
#include "Component.h"

#include <bitset>
#include <vector>

#include "AABB.h"
#include "Material.h"
#include "Mesh.h" // Including MeshRenderer.h usually means you also need Mesh.h

class Model;
class Ray;
struct RaycastHit;
class Texture;

class MeshRenderer : public Component
{
    TYPE_DECL_CHILD();
public:
    MeshRenderer(Actor* actor);
    ~MeshRenderer();
	
    void Render(bool opaque = true, bool translucent = true);

    void SetShader(Shader* shader) { mShader = shader; }

    void SetModel(Model* model);
    Model* GetModel() const { return mModel; }
    std::string GetModelName() const;
    
    void SetMesh(Mesh* mesh);
    void AddMesh(Mesh* mesh);
    const std::vector<Mesh*>& GetMeshes() const { return mMeshes; }
    Mesh* GetMesh(int index) const;
	
	void SetMaterial(int index, Material material);
	Material* GetMaterial(int index);
	Material* GetMaterial(int meshIndex, int submeshIndex);
    std::vector<Material>& GetMaterials() { return mMaterials; }

    void SetVisibility(int meshIndex, int submeshIndex, bool visible);
    
	bool Raycast(const Ray& ray, RaycastHit& hitInfo);

    AABB GetAABB() const;
    void DebugDrawAABBs(const Color32& color = Color32::White, const Color32& meshColor = Color32(255, 255, 132));
    
private:
	// A model, if any was specified.
	// NOT used for rendering (meshes are used directly). But can be helpful to keep around.
	Model* mModel = nullptr;

    // If defined, a shader to use when creating materials.
    // If not defined, the default shader is used (3D-Tex-Unlit).
    Shader* mShader = Material::sDefaultShader;
	
    // A mesh component can render one or more meshes.
    // If more than one is specified, they will be rendered in order.
    std::vector<Mesh*> mMeshes;
    
    // A material describes how to render a mesh.
    // Each mesh *must have* a material!
	// If a mesh has multiple submeshes, each submesh *must have* a material!
    std::vector<Material> mMaterials;

    // Visibility toggles for individual submeshes. Since bitsets are false by default, a true value means the submesh is invisible.
    // Would we ever have more than 64 total submeshes? Guess we'll find out! Should take up 8 bytes per MeshRenderer.
    static const int kMaxSubmeshes = 64;
    std::bitset<kMaxSubmeshes> mSubmeshInvisible;

    int GetIndexFromMeshSubmeshIndexes(int meshIndex, int submeshIndex);
};
