//
// Mesh.cpp
//
// Clark Kromenaker
//
#include "Mesh.h"
#include "GLVertexArray.h"

Mesh::Mesh(uint vertexCount, uint vertexSize, MeshUsage usage)
{
    mVertexArray = new GLVertexArray(vertexCount, vertexSize, usage);
}

Mesh::~Mesh()
{
    delete mVertexArray;
    
    // Delete vertex attribute data.
    delete mPositions;
    delete mColors;
    delete mNormals;
    delete mUV1;
    
    // Delete indexes.
    delete mIndexes;
}

void Mesh::Render()
{
    switch(mRenderMode)
    {
        default:
        case RenderMode::Triangles:
            mVertexArray->DrawTriangles();
            break;
        case RenderMode::TriangleFan:
            mVertexArray->DrawTriangleFans();
            break;
        case RenderMode::Lines:
            mVertexArray->DrawLines();
            break;
    }
}

void Mesh::Render(uint offset, uint count)
{
    switch(mRenderMode)
    {
        default:
        case RenderMode::Triangles:
            mVertexArray->DrawTriangles(offset, count);
            break;
        case RenderMode::TriangleFan:
            mVertexArray->DrawTriangleFans(offset, count);
            break;
        case RenderMode::Lines:
            mVertexArray->DrawLines(offset, count);
            break;
    }
}

void Mesh::SetPositions(const float* positions)
{
    mPositions = positions;
    mVertexArray->SetPositions(positions);
}

void Mesh::SetColors(const float* colors)
{
    mColors = colors;
    mVertexArray->SetColors(colors);
}

void Mesh::SetNormals(const float* normals)
{
    mNormals = normals;
    mVertexArray->SetNormals(normals);
}

void Mesh::SetUV1(const float* uvs)
{
    mUV1 = uvs;
    mVertexArray->SetUV1(uvs);
}

void Mesh::SetIndexes(const ushort* indexes, int count)
{
    mIndexes = indexes;
    mVertexArray->SetIndexes(indexes, count);
}
