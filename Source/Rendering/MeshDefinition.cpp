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

void MeshDefinition::SetIndexData(unsigned int count, unsigned short* data)
{
    indexCount = count;
    indexData = data;
}
