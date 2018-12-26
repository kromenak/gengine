//
// RenderPacket.h
//
// Clark Kromenaker
//
// Contains all the information we need to render an object.
//
#pragma once
#include "Matrix4.h"

class Mesh;
class Material;

struct RenderPacket
{
    // The mesh provides vertex data for rendering.
	Mesh* mesh = nullptr;
    
    // The material provides everything else for rendering.
	Material* materials = nullptr;
	int materialsCount = 0;
    
    // Data required for mesh instancing.
    Matrix4 worldTransform;
    
    void Render();
};
