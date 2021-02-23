//
// MeshRenderer.h
//
// Clark Kromenaker
//
// Component for rendering meshes.
//
#pragma once
#include "Component.h"

#include <vector>

#include "Material.h"

class Mesh;
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
    
    void SetModel(Model* model);
    
    void SetMesh(Mesh* mesh);
    void AddMesh(Mesh* mesh);
	
	void SetMaterial(int index, Material material);
	Material* GetMaterial(int index);
	Material* GetMaterial(int meshIndex, int submeshIndex);
	
	Model* GetModel() const { return mModel; }
	
	const std::vector<Mesh*>& GetMeshes() const { return mMeshes; }
	Mesh* GetMesh(int index) const;
	
	bool Raycast(const Ray& ray, RaycastHit& hitInfo);
	
	void DebugDrawAABBs();
    
private:
	// A model, if any was specified.
	// NOT used for rendering (meshes are used directly). But can be helpful to keep around.
	Model* mModel = nullptr;
	
    // A mesh component can render one or more meshes.
    // If more than one is specified, they will be rendered in order.
    std::vector<Mesh*> mMeshes;
    
    // A material describes how to render a mesh.
    // Each mesh *must have* a material!
	// If a mesh has multiple submeshes, each submesh *must have* a material!
    std::vector<Material> mMaterials;
};
