#include "MeshDefinition.h"

MeshDefinition::MeshDefinition(MeshUsage usage, unsigned int vertexCount) :
    meshUsage(usage),
    vertexCount(vertexCount)
{

}

void MeshDefinition::AddVertexAttribute(const VertexAttribute& attribute)
{
    vertexDefinition.attributes.push_back(attribute);
}

void MeshDefinition::AddVertexData(const VertexAttribute& attribute, void* data)
{
    vertexDefinition.attributes.push_back(attribute);
    vertexData.push_back(data);
}

void MeshDefinition::SetVertexData(void* data)
{
    if(vertexData.size() < 1)
    {
        vertexData.push_back(data);
    }
    else
    {
        vertexData[0] = data;
    }
}

void MeshDefinition::SetIndexData(unsigned int indexCount, unsigned short* indexData)
{
    this->indexCount = indexCount;
    this->indexData = indexData;
}

void* MeshDefinition::GetVertexData(const VertexAttribute& attribute) const
{
    for(int i = 0; i < vertexDefinition.attributes.size(); i++)
    {
        if(vertexDefinition.attributes[i] == attribute)
        {
            return vertexData[i];
        }
    }
    return nullptr;
}