//
// Mesh.cpp
//
// Clark Kromenaker
//
#include "Mesh.h"
#include "GLVertexArray.h"

Mesh::Mesh(uint vertexCount, uint vertexSize, MeshUsage usage) : mVertexCount(vertexCount)
{
    mVertexArray = new GLVertexArray(vertexCount, vertexSize, usage);
}

Mesh::~Mesh()
{
    delete mVertexArray;
    
    // Delete vertex attribute data.
    delete[] mPositions;
    delete[] mColors;
    delete[] mNormals;
    delete[] mUV1;
    
    // Delete indexes.
    delete[] mIndexes;
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

void Mesh::SetPositions(float* positions)
{
    if(mPositions == nullptr)
    {
        mPositions = new float[mVertexCount * 3];
    }
    
    /*
    for(int i = 0; i < mVertexCount * 3; i++)
    {
        if(!Math::AreEqual(mPositions[i], positions[i]))
        {
            std::cout << mPositions[i] << " to " << positions[i] << std::endl;
        }
        //std::cout << mPositions[i] << ", " << mPositions[i + 1] << ", " << mPositions[i + 2] << std::endl;
    }
    */
    
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

void Mesh::SetIndexes(ushort* indexes, int count)
{
    mIndexes = indexes;
    mVertexArray->SetIndexes(indexes, count);
}
