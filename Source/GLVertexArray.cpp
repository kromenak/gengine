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
    
    //glGenBuffers(1, &ibo);
    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    //glBufferData(GL_ELEMENT_ARRAY_BUFFER, 24 * sizeof(GLushort), cube_elements, GL_STATIC_DRAW);

    mVertexCount = vertPositionsCount / 3;
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
}

void GLVertexArray::Draw()
{
    Activate();
    if(mIBO != GL_NONE)
    {
        //TODO: Draw using indexes.
    }
    else
    {
        glDrawArrays(GL_TRIANGLES, 0, mVertexCount);
    }
}
