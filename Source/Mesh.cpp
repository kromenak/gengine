//
// Mesh.cpp
//
// Clark Kromenaker
//
#include "Mesh.h"

#include "GLVertexArray.h"

Mesh::Mesh(unsigned int vertexCount, unsigned int vertexSize, MeshUsage usage) : mVertexCount(vertexCount)
{
    mVertexArray = new GLVertexArray(vertexCount, vertexSize, usage);
}

Mesh::~Mesh()
{
    // Delete vertex attribute data.
    delete[] mPositions;
    delete[] mColors;
    delete[] mNormals;
    delete[] mUV1;
    
    // Delete indexes.
    delete[] mIndexes;
    
    delete mVertexArray;
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

void Mesh::Render(unsigned int offset, unsigned int count)
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

void Mesh::SetPositions(float* positions)
{
    if(mPositions == nullptr)
    {
        mPositions = new float[mVertexCount * 3];
    }
    memcpy(mPositions, positions, mVertexCount * 3 * sizeof(float));
    
    mVertexArray->SetPositions(mPositions);
}

void Mesh::SetColors(float* colors)
{
    mColors = colors;
    mVertexArray->SetColors(colors);
}

void Mesh::SetNormals(float* normals)
{
    mNormals = normals;
    mVertexArray->SetNormals(normals);
}

void Mesh::SetUV1(float* uvs)
{
    mUV1 = uvs;
    mVertexArray->SetUV1(uvs);
}

void Mesh::SetIndexes(unsigned short* indexes, int count)
{
    mIndexes = indexes;
    mVertexArray->SetIndexes(indexes, count);
}
