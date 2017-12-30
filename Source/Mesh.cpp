//
// Mesh.cpp
//
// Clark Kromenaker
//
#include "Mesh.h"
#include "GLVertexArray.h"

Mesh::Mesh()
{
    mVertexArray = new GLVertexArray();
}

Mesh::Mesh(GLfloat* positions, int positionCount)
{
    mVertexArray = new GLVertexArray(positions, positionCount);
}

void Mesh::Render()
{
    if(mVertexArray != nullptr)
    {
        mVertexArray->Draw();
    }
}

void Mesh::SetPositions(const GLfloat* vertPositions, int count)
{
    mVertexArray->SetPositions(vertPositions, count);
}

void Mesh::SetColors(const GLfloat* vertColors, int count)
{
    mVertexArray->SetColors(vertColors, count);
}

void Mesh::SetUV1(const GLfloat* vertUV1s, int count)
{
    mVertexArray->SetUV1(vertUV1s, count);
}

void Mesh::SetIndexes(const GLushort* indexes, int count)
{
    mVertexArray->SetIndexes(indexes, count);
}
