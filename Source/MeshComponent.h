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
class Texture;

class MeshComponent : public Component
{
public:
    MeshComponent(Actor* actor);
    ~MeshComponent();
    
    void Update(float deltaTime) override;
    
    void Render();
    
    void SetModel(Model* model);
    void SetMesh(Mesh* mesh);
    
    void AddTexture(Texture* texture);
    
private:
    // A mesh component can render one or more meshes.
    // If more than one is specified, they will be rendered in order.
    std::vector<Mesh*> mMeshes;
    
    // Textures to pass to the shader.
    // If more than one is specified, they will be passed in order.
    std::vector<Texture*> mTextures;
};
