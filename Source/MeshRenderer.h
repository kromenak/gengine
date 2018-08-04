//
// MeshRenderer.h
//
// Clark Kromenaker
//
// Component for rendering meshes.
//
#pragma once
#include <vector>
#include "Component.h"
#include "Material.h"
#include "RenderPacket.h"

class Mesh;
class Model;
class Texture;

class MeshRenderer : public Component
{
public:
    MeshRenderer(Actor* actor);
    ~MeshRenderer();
    
    void Render();
    std::vector<RenderPacket> GetRenderPackets();
    
    void SetModel(Model* model);
    
    void SetMesh(Mesh* mesh);
    void AddMesh(Mesh* mesh);
    
private:
    // A mesh component can render one or more meshes.
    // If more than one is specified, they will be rendered in order.
    std::vector<Mesh*> mMeshes;
    
    // A material describes how to render a mesh.
    // Each mesh *must have* a material!
    std::vector<Material> mMaterials;
};
