//
// GLVertexArray.h
//
// Clark Kromenaker
//
// Description goes here!
//
#pragma once
#include <GL/glew.h>

// Some OpenGL calls take in array indexes/offsets as pointers.
// This macro just makes the syntax clearer for the reader.
#define BUFFER_OFFSET(i) ((char *)NULL + (i))

class GLVertexArray
{
public:
    GLVertexArray() { };
    GLVertexArray(const GLfloat* vertPositions, int vertPositionsCount);
    GLVertexArray(const GLfloat* vertPositions, int vertPositionsCount,
                  const GLushort* indexes, int indexesCount);
    ~GLVertexArray();
    
    void SetPositions(const GLfloat* vertPositions, int count);
    void SetColors(const GLfloat* vertColors, int count);
    void SetUV1(const GLfloat* uvs, int count);
    void SetIndexes(const GLushort* indexes, int count);
    void Build();
    
    void Activate();
    void Draw();
    void Draw(int offset, int count);
    
private:
    // Buffer objects for OpenGL - Vertex Buffer Object,
    // Vertex Array Object, and Index Buffer Object.
    GLuint mVBO = GL_NONE;
    GLuint mVAO = GL_NONE;
    GLuint mIBO = GL_NONE;
    
    // Position data.
    const GLfloat* mPositions = nullptr;
    int mPositionCount = 0;
    
    // Color data.
    const GLfloat* mColors = nullptr;
    int mColorCount = 0;
    
    // UV data.
    const GLfloat* mUV1 = nullptr;
    int mUV1Count = 0;
    
    // Index data.
    const GLushort* mIndexes = nullptr;
    int mIndexCount = 0;
};
