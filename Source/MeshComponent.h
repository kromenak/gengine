//
// MeshComponent.h
//
// Clark Kromenaker
//
// Component for rendering meshes.
//
#pragma once
#include <vector>
#include "Component.h"

class Mesh;

class MeshComponent : public Component
{
public:
    MeshComponent(Actor* actor);
    ~MeshComponent();
    
    void Update(float deltaTime) override;
    
    void Render();
    
    void SetModel(Model* model);
    void SetMesh(Mesh* mesh);
    
private:
    // A mesh component can render one or more meshes.
    // If more than one is specified, they will be rendered in order.
    std::vector<Mesh*> mMeshes;
};
