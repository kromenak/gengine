//
// Mesh.cpp
//
// Clark Kromenaker
//
#include "Mesh.h"
#include "GLVertexArray.h"

Mesh::Mesh(GLfloat* positions, int positionCount)
{
    mVertexArray = new GLVertexArray(positions, positionCount);
}
