//
// GLVertexArray.h
//
// Clark Kromenaker
//
// A chunk of geometry, potentially indexed, that can be rendered by OpenGL.
//
#pragma once
#include <GL/glew.h>

#include "AtomicTypes.h"
#include "Mesh.h"

class GLVertexArray
{
public:
    GLVertexArray(unsigned int vertexCount, unsigned int vertexSize, MeshUsage usage);
    ~GLVertexArray();
    
    void SetPositions(float* positions);
    void SetColors(float* colors);
    void SetNormals(float* normals);
    void SetUV1(float* uvs);
    
    void SetIndexes(unsigned short* indexes, unsigned int count);
    
    void DrawTriangles();
    void DrawTriangles(unsigned int offset, unsigned int count);
    
    void DrawTriangleFans();
    void DrawTriangleFans(unsigned int offset, unsigned int count);
    
    void DrawLines();
    void DrawLines(unsigned int offset, unsigned int count);
    
    void Draw(GLenum mode);
    void Draw(GLenum mode, unsigned int offset, unsigned int count);
    
private:
    // Number of vertices in the mesh.
    unsigned int mVertexCount = 0;
    
    // The VBO (vertex buffer object) holds all per-vertex data (position, color, normals, etc).
    GLuint mVBO = GL_NONE;
    
    // The IBO (index buffer object) holds index values for indexed geometry.
    // It's optional, but improves performance.
    GLuint mIBO = GL_NONE;
    
    // The VAO (vertex array object) provides mapping info for the VBO.
    // The VBO is just a big chunk of memory. The VAO dictates what chunk holds
    // positions vs. colors vs whatever, and whether the data is separate or interleaved.
    GLuint mVAO = GL_NONE;
    
    // Indicates that we need to update some aspect of the VBO.
    unsigned int mVboUpdateMask = 0;
    
    // Vertex data
    float* mPositions = nullptr;
    float* mColors = nullptr;
    float* mNormals = nullptr;
    float* mUV1 = nullptr;
    
    // Index data
    unsigned short* mIndexes = nullptr;
    unsigned int mIndexCount = 0;
    
    void BuildVBO();
    void UpdateVBO();
};
