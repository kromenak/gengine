//
// Mesh.cpp
//
// Clark Kromenaker
//
#include "Mesh.h"

#include <cassert>

Mesh::Mesh()
{
	
}

Mesh::Mesh(unsigned int vertexCount, unsigned int vertexSize, MeshUsage usage)
{
	// Since a mesh should REALLY REALLY have at least one submesh, this constructor
	// allows us to create a submesh immediately upon construction.
	mSubmeshes.push_back(new Submesh(vertexCount, vertexSize, usage));
}

Mesh::~Mesh()
{
	for(auto& submesh : mSubmeshes)
	{
		delete submesh;
	}
}

void Mesh::Render()
{
	for(auto& submesh : mSubmeshes)
	{
		submesh->Render();
	}
}

void Mesh::Render(unsigned int submeshIndex)
{
	assert(submeshIndex < mSubmeshes.size());
	mSubmeshes[submeshIndex]->Render();
}

void Mesh::Render(unsigned int submeshIndex, unsigned int offset, unsigned int count)
{
	assert(submeshIndex < mSubmeshes.size());
	mSubmeshes[submeshIndex]->Render(offset, count);
}

bool Mesh::Raycast(const Ray& ray)
{
	//TODO: Could do a bounds check first, before checking against all triangles!
	// Pass through to each submesh...
	for(auto& submesh : mSubmeshes)
	{
		if(submesh->Raycast(ray))
		{
			return true;
		}
	}
	return false;
}
