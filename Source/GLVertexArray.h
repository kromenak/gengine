//
// GLVertexArray.h
//
// Clark Kromenaker
//
// A chunk of geometry, potentially indexed, that can be rendered by OpenGL.
//
#pragma once
#include <GL/glew.h>
#include "Types.h"
#include "Mesh.h"

class GLVertexArray
{
public:
    GLVertexArray(uint vertexCount, uint vertexSize, MeshUsage usage);
    ~GLVertexArray();
    
    void SetPositions(const float* positions);
    void SetColors(const float* colors);
    void SetNormals(const float* normals);
    void SetUV1(const float* uvs);
    
    void SetIndexes(const ushort* indexes, uint count);
    
    void DrawTriangles();
    void DrawTriangles(uint offset, uint count);
    
    void DrawTriangleFans();
    void DrawTriangleFans(uint offset, uint count);
    
    void DrawLines();
    void DrawLines(uint offset, uint count);
    
    void Draw(GLenum mode);
    void Draw(GLenum mode, uint offset, uint count);
    
private:
    // Number of vertices in the mesh.
    uint mVertexCount = 0;
    
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
    uint mVboUpdateMask = 0;
    
    // Vertex data
    const float* mPositions = nullptr;
    const float* mColors = nullptr;
    const float* mNormals = nullptr;
    const float* mUV1 = nullptr;
    
    // Index data
    const ushort* mIndexes = nullptr;
    uint mIndexCount = 0;
    
    void BuildVBO();
    void UpdateVBO();
};
