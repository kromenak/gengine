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
class Texture;

class MeshRenderer : public Component
{
    TYPE_DECL_CHILD();
public:
    MeshRenderer(Actor* actor);
    ~MeshRenderer();
	
	void RenderOpaque();
	void RenderTranslucent();
    
    void SetModel(Model* model);
    
    void SetMesh(Mesh* mesh);
    void AddMesh(Mesh* mesh);
	
	void SetMaterial(int index, Material material);
	
	Model* GetModel() const { return mModel; }
    std::vector<Mesh*> GetMeshes() const { return mMeshes; }
	
	Matrix4 GetMeshWorldTransform(int index) const;
    
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
