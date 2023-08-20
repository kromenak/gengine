//
// Clark Kromenaker
//
// Struct for defining the layout and data for a mesh.
// Once defined, this data can be sent to the GPU to actually render the thing.
//
#pragma once
#include <cstddef>
#include <utility>

#include "Value.h"
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

    // Vertex data.
    // For interleaved data, there should be a single element: a pointer to a list of vertex structs.
    // For packed data, there should be one element per attribute: pointers to each attribute data.
    std::vector<BasicValue> vertexData;

    // A buffer of indexes, if mesh uses indexes.
    unsigned short* indexData = nullptr;

    // Vertex/index data is usually "owned" by this object.
    // But in rare circumstances, data lifetime may be owned by external systems - so keep track of that here!
    bool ownsData = true;
    
    MeshDefinition() = default;
    MeshDefinition(MeshUsage usage, unsigned int vertexCount);

    void SetVertexLayout(VertexLayout layout) { vertexDefinition.layout = layout; }

    // For packed data: specify attribute and data together.
    template<typename T> void AddVertexData(const VertexAttribute& attribute, T* data);

    // For interleaved data: specify attributes separately from singular data buffer.
    void AddVertexAttribute(const VertexAttribute& attribute);
    template<typename T> void SetVertexData(T* data);
    
    void SetIndexData(unsigned int count, unsigned short* data);

    template<typename T> T* GetVertexData(const VertexAttribute& attribute) const;
};

template<typename T>
void MeshDefinition::AddVertexData(const VertexAttribute& attribute, T* data)
{
    vertexDefinition.attributes.push_back(attribute);
    vertexData.emplace_back(std::forward<T*>(data));
}

template<typename T>
void MeshDefinition::SetVertexData(T* data)
{
    // "Set" overwrites all existing data.
    if(ownsData)
    {
        for(auto& data : vertexData)
        {
            data.DeleteArray();
        }
    }
    vertexData.clear();
    
    // Set data as only element.
    vertexData.emplace_back(std::forward<T*>(data));
}

template<typename T>
T* MeshDefinition::GetVertexData(const VertexAttribute& attribute) const
{
    for(size_t i = 0; i < vertexDefinition.attributes.size(); i++)
    {
        if(vertexDefinition.attributes[i] == attribute)
        {
            return reinterpret_cast<T*>(vertexData[i].data);
        }
    }
    return nullptr;
}
