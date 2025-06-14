//
// Clark Kromenaker
//
// Renderable 3D geometry. Consists of one or more Submeshes - each Submesh
// contains the actual vertex data to be rendered.
//
// Also contains some data shared by all Submeshes.
//
#pragma once
#include <vector>

#include "AABB.h"
#include "Matrix4.h"
#include "Submesh.h"
#include "Vector2.h"

class Ray;

class Mesh
{
public:
    Mesh() = default;
    ~Mesh();

    void Render();
    void Render(unsigned int submeshIndex);
    void Render(unsigned int submeshIndex, unsigned int offset, unsigned int count);

    void SetMeshToLocalMatrix(const Matrix4& mat) { mMeshToLocalMatrix = mat; }
    Matrix4& GetMeshToLocalMatrix() { return mMeshToLocalMatrix; }

    void SetAABB(const AABB& aabb) { mAABB = aabb; }
    const AABB& GetAABB() const { return mAABB; }

    Submesh* AddSubmesh(const MeshDefinition& meshDefinition);

    Submesh* GetSubmesh(int index) const { return index >= 0 && index < static_cast<int>(mSubmeshes.size()) ? mSubmeshes[index] : nullptr; }
    int GetSubmeshCount() const { return static_cast<int>(mSubmeshes.size()); }

    const std::vector<Submesh*>& GetSubmeshes() const { return mSubmeshes; }

    bool Raycast(const Ray& ray, float& outRayT, int& outSubmeshIndex, Vector2& outUV);

private:
    // A mesh is really just a container for one or more submeshes.
    std::vector<Submesh*> mSubmeshes;

    // Transforms from "mesh space" to "local space".
    // Each Mesh in GK3 has its own position/rotation/scale, and Submesh vertices are relative to the Mesh coordinate system.
    // This matrix represents the Mesh's coordinate system and can be used to transform from mesh space to parent space.
    Matrix4 mMeshToLocalMatrix = Matrix4::Identity;

    // An AABB for the mesh, in its own local space.
    AABB mAABB;
};
