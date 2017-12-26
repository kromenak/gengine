//
// Mesh.h
//
// Clark Kromenaker
//
// Represents a renderable piece of 3D geometry.
// Vertex data - positions, colors, UVs, etc.
//
#pragma once
#include <GL/glew.h>

class GLVertexArray;

class Mesh
{
public:
    Mesh(GLfloat* positions, int positionCount);
    
    GLVertexArray* GetVertexArray() const { return mVertexArray; }
    
private:
    GLVertexArray* mVertexArray = nullptr;
};
