#include "Mesh.h"

#include <cassert>

#include "Collisions.h"

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

Submesh* Mesh::AddSubmesh(const MeshDefinition& meshDefinition)
{
    Submesh* submesh = new Submesh(meshDefinition);
    mSubmeshes.push_back(submesh);
    return submesh;
}

bool Mesh::Raycast(const Ray& ray, float& outRayT, int& outSubmeshIndex, Vector2& outUV)
{
    // Ensure t value is at default.
    outRayT = FLT_MAX;

    // Before we check per-triangle collision, do a broader AABB check.
    // If we don't hit the AABB, then we shouldn't hit any triangle.
    float aabbT = FLT_MAX;
    if(Intersect::TestRayAABB(ray, mAABB, aabbT))
    {
        // If hit the AABB, do a per-triangle check as well for more precise detection.
        // For example, Gabe's AABBs are pretty rough, so you can select him when clicking nowhere near him (a foot left of his arm).
        // This isn't how the original game works, so I think they must do a per-triangle check as well.
        for(int i = 0; i < mSubmeshes.size(); ++i)
        {
            float submeshRayT = FLT_MAX;
            if(mSubmeshes[i]->Raycast(ray, submeshRayT, outUV))
            {
                if(submeshRayT < outRayT)
                {
                    outRayT = submeshRayT;
                    outSubmeshIndex = i;
                }
            }
        }
    }

    // Either didn't hit AABB or did hit AABB, but failed triangle test.
    return outRayT < FLT_MAX;
}
