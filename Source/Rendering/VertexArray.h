//
// Clark Kromenaker
//
// A chunk of geometry that can be rendered by the GPU.
// The lowest-level rendering object you can use.
//
// The VA creates/owns GPU resources.
// 
// It also allocates and manages memory for vertex/index data.
// You can assume any data passed to VA is copied internally - so buffers do not need to be dynamically allocated.
//
#pragma once
#include <GL/glew.h>

#include "MeshDefinition.h"

class VertexArray
{
public:
    VertexArray() = default;
    VertexArray(const MeshDefinition& data);
    ~VertexArray();
    
    // VertexArrays contain handles to GPU resources, so don't allow copying!
    VertexArray(const VertexArray& other) = delete;
    VertexArray& operator=(const VertexArray& other) = delete;
    
    // VertexArrays can be moved to/from.
    VertexArray(VertexArray&& other);
    VertexArray& operator=(VertexArray&& other);
    
    void DrawTriangles();
    void DrawTriangles(unsigned int offset, unsigned int count);
    
    void DrawTriangleStrips();
    void DrawTriangleStrips(unsigned int offset, unsigned int count);
    
    void DrawTriangleFans();
    void DrawTriangleFans(unsigned int offset, unsigned int count);
    
    void DrawLines();
    void DrawLines(unsigned int offset, unsigned int count);
    
    void Draw(GLenum mode);
    void Draw(GLenum mode, unsigned int offset, unsigned int count);

    unsigned int GetVertexCount() const { return mData.vertexCount; }
    unsigned int GetIndexCount() const { return mData.indexCount; }

    void ChangeVertexData(void* data);
    void ChangeVertexData(VertexAttribute::Semantic semantic, void* data);

    void ChangeIndexData(unsigned short* indexes);
    void ChangeIndexData(unsigned short* indexes, unsigned int count);
    
private:
    // Mesh data (vertices, normals, indexes, etc).
    // This is maintained and holds CPU-side copies of all mesh data.
    MeshDefinition mData;
    
    // The VBO (vertex buffer object) holds all mesh vertex data (position, color, normals, etc).
    GLuint mVBO = GL_NONE;

    // The VAO (vertex array object) provides mapping info for the VBO.
    // The VBO is just a big chunk of memory. The VAO dictates how to interpret the memory to read vertex data.
    GLuint mVAO = GL_NONE;
    
    // The IBO (index buffer object) holds index values for indexed geometry.
    // It's optional, but improves performance.
    GLuint mIBO = GL_NONE;
    
    void CreateVBO();
    void CreateVAO();
    void RefreshIBOContents();
};
