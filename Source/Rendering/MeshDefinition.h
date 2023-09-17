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
    // Use "dynamic" if you expect to be modifying the contents of the vertex buffer frequently.
    MeshUsage meshUsage = MeshUsage::Static;

    // Defines the vertex data ordering and layout for this mesh.
    VertexDefinition vertexDefinition;

    // Number of vertices/indices in this mesh.
    unsigned int vertexCount = 0;
    unsigned int indexCount = 0;

    // Vertex data.
    // For interleaved data, there should be a single element: a pointer to a list of vertex structs.
    // For packed data, there should be one element per attribute: pointers to each attribute data.
    std::vector<void*> vertexData;

    // Because vertex data can be ANYTHING (floats, structs, etc), we need to know how to delete that data if we own it.
    // The type handlers in this list mirror the vertex data list, and know how to delete the data at the same index over there.
    std::vector<TypeHandler*> vertexDataTypeHandlers;

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
    void ClearVertexData();

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

    // Store the data, as well as info on how to delete this data later.
    vertexData.emplace_back(std::forward<T*>(data));
    vertexDataTypeHandlers.emplace_back(GetTypeHandler<T>());
}

template<typename T>
void MeshDefinition::SetVertexData(T* data)
{
    // "Set" overwrites all existing data.
    ClearVertexData();
    
    // Set data as only element.
    vertexData.emplace_back(std::forward<T*>(data));
    vertexDataTypeHandlers.emplace_back(GetTypeHandler<T>());
}

template<typename T>
T* MeshDefinition::GetVertexData(const VertexAttribute& attribute) const
{
    for(size_t i = 0; i < vertexDefinition.attributes.size(); i++)
    {
        if(vertexDefinition.attributes[i] == attribute)
        {
            return reinterpret_cast<T*>(vertexData[i]);
        }
    }
    return nullptr;
}
