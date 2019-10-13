//
// Mesh.h
//
// Clark Kromenaker
//
// Renderable 3D geometry. Consists of one or more Submeshes - each Submesh
// contains the actual vertex data to be rendered.
//
// Also contains some data shared by all Submeshes.
//
#pragma once
#include <GL/glew.h>
//#include <OpenGL/gl.h>
#include <vector>

#include "Matrix4.h"
#include "Quaternion.h"
#include "Submesh.h"
#include "Vector3.h"

class Mesh
{
public:
	Mesh();
	Mesh(unsigned int vertexCount, unsigned int vertexSize, MeshUsage usage);
    ~Mesh();
	
    void Render();
	void Render(unsigned int submeshIndex);
	void Render(unsigned int submeshIndex, unsigned int offset, unsigned int count);
    
    void SetLocalTransformMatrix(const Matrix4& mat) { mLocalTransformMatrix = mat; }
    Matrix4& GetLocalTransformMatrix() { return mLocalTransformMatrix; }
	
	void AddSubmesh(Submesh* submesh) { mSubmeshes.push_back(submesh); }
	const std::vector<Submesh*>& GetSubmeshes() const { return mSubmeshes; }
	Submesh* GetSubmesh(int index) const { return index >= 0 && index < mSubmeshes.size() ? mSubmeshes[index] : nullptr; }
	int GetSubmeshCount() const { return static_cast<int>(mSubmeshes.size()); }
	
private:
	std::vector<Submesh*> mSubmeshes;
	
    // Matrix containing local translation/rotation.
    Matrix4 mLocalTransformMatrix;
};
