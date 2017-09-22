//
// GLVertexArray.cpp
//
// Clark Kromenaker
//

#include "GLVertexArray.h"

GLVertexArray::GLVertexArray(const GLfloat* vertPositions, int vertPositionsCount)
{
    glGenBuffers(1, &mVBO);
    glBindBuffer(GL_ARRAY_BUFFER, mVBO);
    glBufferData(GL_ARRAY_BUFFER, vertPositionsCount * sizeof(GLfloat), vertPositions, GL_STATIC_DRAW);
    
    glGenVertexArrays(1, &mVAO);
    glBindVertexArray(mVAO);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, mVBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    
    mVertexCount = vertPositionsCount / 3;
}

GLVertexArray::GLVertexArray(const GLfloat* vertPositions, int vertPositionsCount,
                             const GLushort* indexes, int indexesCount)
: GLVertexArray(vertPositions, vertPositionsCount)
{
    glGenBuffers(1, &mIBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexesCount * sizeof(GLushort), indexes, GL_STATIC_DRAW);
    mIndexesCount = indexesCount;
}

GLVertexArray::~GLVertexArray()
{
    if(mVBO != GL_NONE)
    {
        glDeleteBuffers(1, &mVBO);
    }
    if(mVAO != GL_NONE)
    {
        glDeleteVertexArrays(1, &mVAO);
    }
    if(mIBO != GL_NONE)
    {
        glDeleteBuffers(1, &mIBO);
    }
}

void GLVertexArray::Activate()
{
    glBindVertexArray(mVAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIBO);
}

void GLVertexArray::Draw()
{
    Activate();
    if(mIBO != GL_NONE)
    {
        glDrawElements(GL_TRIANGLES, mIndexesCount, GL_UNSIGNED_SHORT, (void*)0);
    }
    else
    {
        glDrawArrays(GL_TRIANGLES, 0, mVertexCount);
    }
}
