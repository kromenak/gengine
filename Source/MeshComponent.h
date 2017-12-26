//
// MeshComponent.h
//
// Clark Kromenaker
//
// Component for rendering meshes.
//
#pragma once
#include "Component.h"

class Mesh;

class MeshComponent : public Component
{
public:
    MeshComponent(Actor* actor);
    ~MeshComponent();
    
    void Render();
    
    void SetMesh(Mesh* mesh) { mMesh = mesh; }
    
private:
    Mesh* mMesh = nullptr;
};
