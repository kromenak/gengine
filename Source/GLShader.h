//
// GLShader.h
//
// Clark Kromenaker
//
// Description goes here!
//

#pragma once
#include <GL/glew.h>

class GLShader
{
public:
    //GLShader(const char* shaderName);
    GLShader(const char* vertShaderPath, const char* fragShaderPath);
    ~GLShader();
    
    bool IsGood();
    
    void Activate();
    
private:
    GLuint LoadAndCompileShaderFromFile(const char* filePath, GLuint shaderType);
    bool IsShaderCompiled(GLuint shader);
    bool IsProgramLinked(GLuint program);
    
private:
    // Handle to the compiled and linked GL shader program.
    GLuint mProgram = GL_NONE;
    
    // Did we encounter an error during compile/linking?
    bool mError = false;
};
