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
#include "Vector3.h"
#include "Quaternion.h"

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
    
    void SetOffset(Vector3 offset) { mOffset = offset; }
    Vector3 GetOffset() { return mOffset; }
    
    void SetRotation(Quaternion rotation) { mRotation = rotation; }
    Quaternion GetRotation() { return mRotation; }
    
    GLVertexArray* GetVertexArray() const { return mVertexArray; }
    
private:
    // An offset value, to be passed to the shader.
    Vector3 mOffset;
    Quaternion mRotation;
    
    // Vertex array object that actually renders the thing.
    GLVertexArray* mVertexArray = nullptr;
};
