//
// Clark Kromenaker
//
// Struct for defining the layout and data for a mesh.
// Once defined, this data can be sent to the GPU to actually render the thing.
//
#pragma once
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

    // Number of vertices/indices in this mesh.
    unsigned int vertexCount = 0;
    unsigned int indexCount = 0;

    // Pointers to vertex data.
    // For interleaved data, there should be a single element: a pointer to a list of vertex structs.
    // For packed data, there should be one element per attribute: pointers to each attribute data.
    std::vector<void*> vertexData;

    // A buffer of indexes, if mesh uses indexes.
    unsigned short* indexData = nullptr;

    // Vertex/index data is usually "owned" by this object.
    // But in rare circumstances, data lifetime may be owned by external systems - so keep track of that here!
    bool ownsData = true;
    
    MeshDefinition() = default;
    MeshDefinition(MeshUsage usage, unsigned int vertexCount);

    void SetVertexLayout(VertexLayout layout) { vertexDefinition.layout = layout; }

    // For packed data: specify attribute and data together.
    void AddVertexData(const VertexAttribute& attribute, void* data);

    // For interleaved data: specify attributes separately from singular data buffer.
    void AddVertexAttribute(const VertexAttribute& attribute);
    void SetVertexData(void* data);
    
    void SetIndexData(unsigned int indexCount, unsigned short* indexData);

    void* GetVertexData(const VertexAttribute& attribute) const;
};