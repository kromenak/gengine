//
// VertexArray.h
//
// Clark Kromenaker
//
// A chunk of geometry that can be rendered by the GPU.
// The lowest-level rendering object you can use.
//
// Vertex data sent to a VA is copied to the GPU right away, so the source data
// can be discarded after creation (unless it needs to change over time).
//
// The VA owns its handles to the GPU resources, but it does not own any
// allocated memory for vertex data or index data.
//
#pragma once
#include <GL/glew.h>

#include "VertexDefinition.h"

enum class MeshUsage
{
    Static,
    Dynamic
};

struct MeshDefinition
{
    // Expected usage for this mesh.
    // Use "dynamic" if you expect to be modifying the contents of the VA frequently.
    MeshUsage meshUsage = MeshUsage::Static;
    
    // Defines the vertex data ordering and layout for this mesh.
    VertexDefinition vertexDefinition;
    
    // Vertex and index counts.
    // Important that these are set accurately, as buffer sizes are derived from these.
    unsigned int vertexCount = 0;
    unsigned int indexCount = 0;
    
    // Pointer to ALL vertex data. Note that this pointer SHOULD NOT be considered valid after VertexArray construction.
    // For interleaved data, this should be a contiguous block of vertex structs
    // For tightly packed data, this should be a contiguous block of "pointers to pointers" for each attribute's data.
    void* vertexData = nullptr;
    
    // If indexed, pointer to vertex data. Note that this pointer SHOULD NOT be considered valid after VertexArray construction.
    unsigned short* indexData = nullptr;
};

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
    
    void ChangeVertexData(void* data);
    void ChangeVertexData(VertexAttribute::Semantic semantic, void* data);
    
    void ChangeIndexData(unsigned short* indexes, unsigned int count);
    
    void DrawTriangles() const;
    void DrawTriangles(unsigned int offset, unsigned int count) const;
    
    void DrawTriangleStrips() const;
    void DrawTriangleStrips(unsigned int offset, unsigned int count) const;
    
    void DrawTriangleFans() const;
    void DrawTriangleFans(unsigned int offset, unsigned int count) const;
    
    void DrawLines() const;
    void DrawLines(unsigned int offset, unsigned int count) const;
    
    void Draw(GLenum mode) const;
    void Draw(GLenum mode, unsigned int offset, unsigned int count) const;
    
private:
    // Definition data passed in.
    // Note that vertex/index data pointers SHOULD NOT be considered valid after construction!
    MeshDefinition mData;
    
    // The VBO (vertex buffer object) holds all mesh vertex data (position, color, normals, etc).
    GLuint mVBO = GL_NONE;
    
    // The IBO (index buffer object) holds index values for indexed geometry.
    // It's optional, but improves performance.
    GLuint mIBO = GL_NONE;
    
    // The VAO (vertex array object) provides mapping info for the VBO.
    // The VBO is just a big chunk of memory. The VAO dictates how to interpret the memory to read vertex data.
    GLuint mVAO = GL_NONE;
    
    void RefreshIBOContents(unsigned short* indexData, int indexCount);
};
