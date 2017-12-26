//
// GLVertexArray.cpp
//
// Clark Kromenaker
//
#include "GLVertexArray.h"

GLVertexArray::GLVertexArray(const GLfloat* vertPositions, int vertPositionsCount)
{
    // Generate one vertex buffer object and save handle.
    glGenBuffers(1, &mVBO);
    
    // Binds the buffer to intake vertex attribute data.
    glBindBuffer(GL_ARRAY_BUFFER, mVBO);
    
    // Create buffer of a certain size and pass position data into the buffer.
    glBufferData(GL_ARRAY_BUFFER, vertPositionsCount * sizeof(GLfloat), vertPositions, GL_STATIC_DRAW);
    
    // Generate one vertex array object and save handle.
    glGenVertexArrays(1, &mVAO);
    
    // Bind the vertex array so we can perform operations on it.
    glBindVertexArray(mVAO);
    
    // Enable attribute zero, which will hold position data.
    glEnableVertexAttribArray(0);
    
    // Bind the vertex buffer object again.
    glBindBuffer(GL_ARRAY_BUFFER, mVBO);
    
    // Indicates that vertex attributes at index 0 (position data)
    // are 3 values (XYZ) per attribute, float value, not normalized,
    // tightly packed, and no offset.
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    
    // Save the number of vertices. Since each vertex has three
    // position values (XYZ), the number of vertices is position count divided by three.
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
