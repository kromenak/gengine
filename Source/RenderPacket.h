//
// RenderPacket.h
//
// Clark Kromenaker
//
// Contains all the information we need to render an object.
//
#pragma once
#include "Material.h"
#include "Mesh.h"

struct RenderPacket
{
    // The mesh provides vertex data for rendering.
    Mesh* mesh;
    
    // The material provides everything else for rendering.
    Material material;
    
    // Data required for mesh instancing.
    Matrix4 worldTransform;
    
    void Render();
};
