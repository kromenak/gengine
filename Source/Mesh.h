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
    Mesh();
    Mesh(GLfloat* positions, int positionCount);
    
    void Render();
    
    void SetPositions(const GLfloat* vertPositions, int count);
    void SetColors(const GLfloat* vertColors, int count);
    void SetUV1(const GLfloat* vertUV1s, int count);
    void SetIndexes(const GLushort* indexes, int count);
    
    GLVertexArray* GetVertexArray() const { return mVertexArray; }
    
private:
    GLVertexArray* mVertexArray = nullptr;
};
