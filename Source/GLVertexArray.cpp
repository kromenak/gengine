//
// GLVertexArray.cpp
//
// Clark Kromenaker
//
#include "GLVertexArray.h"

GLVertexArray::GLVertexArray(const GLfloat* vertPositions, int vertPositionsCount)
{
    SetPositions(vertPositions, vertPositionsCount);
}

GLVertexArray::GLVertexArray(const GLfloat* vertPositions, int vertPositionsCount,
                             const GLushort* indexes, int indexesCount)
                            : GLVertexArray(vertPositions, vertPositionsCount)
{
    SetIndexes(indexes, indexesCount);
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

void GLVertexArray::SetPositions(const GLfloat* vertPositions, int count)
{
    mPositions = vertPositions;
    mPositionCount = count;
}

void GLVertexArray::SetColors(const GLfloat* vertColors, int count)
{
    mColors = vertColors;
    mColorCount = count;
}

void GLVertexArray::SetIndexes(const GLushort* indexes, int count)
{
    mIndexes = indexes;
    mIndexCount = count;
}

void GLVertexArray::Build()
{
    //TODO: Maybe early out if at least positions aren't set?
    
    // Generate a vertex buffer object and bind it to array buffer target.
    // The ARRAY_BUFFER target is used to upload vertex attribute data (positions, colors, etc).
    glGenBuffers(1, &mVBO);
    glBindBuffer(GL_ARRAY_BUFFER, mVBO);
    
    // Determine size of buffer to create. This depends on how many vertex
    // attributes we need to pass to the shader.
    int positionSize = mPositionCount * sizeof(GLfloat);
    int colorSize = mColorCount * sizeof(GLfloat);
    int bufferSize = positionSize + colorSize;
    
    // Create buffer of needed size, but don't initialize data yet.
    // We'll initialize using subdata command below.
    // TODO: best "usage" to pass here? should this be specified by caller?
    glBufferData(GL_ARRAY_BUFFER, bufferSize, NULL, GL_STATIC_DRAW);
    
    // Fill in the created buffer with our data.
    glBufferSubData(GL_ARRAY_BUFFER, 0, positionSize, mPositions);
    if(mColors != nullptr)
    {
        glBufferSubData(GL_ARRAY_BUFFER, positionSize, colorSize, mColors);
    }
    
    // Generate and bind vertex array object.
    glGenVertexArrays(1, &mVAO);
    glBindVertexArray(mVAO);
    
    // Enable attributes.
    // 0 will hold position data.
    // 1 will hold color data.
    glEnableVertexAttribArray(0);
    if(mColors != nullptr)
    {
        glEnableVertexAttribArray(1);
    }
    
    // Indicates that vertex attributes at index 0 (position data)
    // are 3 values (XYZ) per attribute, float value, not normalized,
    // tightly packed, and no offset.
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    if(mColors != nullptr)
    {
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_TRUE, 0, BUFFER_OFFSET(positionSize));
    }
    
    // Also build the index buffer object if needed.
    if(mIndexes != nullptr)
    {
        glGenBuffers(1, &mIBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, mIndexCount * sizeof(GLushort), mIndexes, GL_STATIC_DRAW);
    }
}

void GLVertexArray::Activate()
{
    // If not yet built, build!
    if(mVAO == GL_NONE)
    {
        Build();
    }
    
    // Bind vertex array and buffer.
    // It's OK if the mIBO is NONE here - will have no effect.
    glBindVertexArray(mVAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIBO);
}

void GLVertexArray::Draw()
{
    // Make sure we are activated first.
    Activate();
    
    // Draw the vertices. Draw mode depends on whether
    // we are using indexed geometry or not.
    if(mIBO != GL_NONE)
    {
        glDrawElements(GL_TRIANGLES, mIndexCount, GL_UNSIGNED_SHORT, BUFFER_OFFSET(0));
    }
    else
    {
        glDrawArrays(GL_TRIANGLES, 0, mPositionCount / 3);
    }
}
