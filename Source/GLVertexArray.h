//
// GLVertexArray.h
//
// Clark Kromenaker
//
// Description goes here!
//

#pragma once
#include <GL/glew.h>

class GLVertexArray
{
public:
    GLVertexArray(const GLfloat* vertPositions, int vertPositionsCount);
    ~GLVertexArray();
    
    void Activate();
    void Draw();
    
private:
    GLuint mVBO = GL_NONE;
    GLuint mVAO = GL_NONE;
    GLuint mIBO = GL_NONE;
    
    int mVertexCount = 0;
};
