//
// Mesh.cpp
//
// Clark Kromenaker
//
#include "Mesh.h"

#include <cassert>

#include "Collisions.h"

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

bool Mesh::Raycast(const Ray& ray, RaycastHit& hitInfo)
{
	// Check against Mesh's AABB to see if we hit it.
	if(Collisions::TestRayAABB(ray, mAABB, hitInfo))
	{
		// If hit the AABB, do a per-triangle check as well for more precise detection.
		// For example, Gabe's AABBs are pretty rough, so you can select him when clicking nowhere near him (a foot left of his arm).
		// This isn't how the original game works, so I think they must do a per-triangle check as well.
		for(auto& submesh : mSubmeshes)
		{
			if(submesh->Raycast(ray))
			{
				return true;
			}
		}
	}
	
	// Either didn't hit AABB or did hit AABB, but failed triangle test.
	return false;
}
