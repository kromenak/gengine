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
	// A mesh will have one or more submeshes, which actually have the vertex data.
	Mesh* mesh = nullptr;
    
    // The material provides everything else for rendering.
	// There should be one material for each submesh, unless all submeshes share a material.
	Material* materials = nullptr;
	int materialsCount = 0;
    
    // Data required for mesh instancing.
    Matrix4 worldTransform;
	
	bool IsTransparent();
	
    void Render();
};
