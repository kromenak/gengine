//
// GLShader.h
//
// Clark Kromenaker
//
// Description goes here!
//

#pragma once
#include <string>
#include <GL/glew.h>

class Matrix4;

class GLShader
{
public:
    //GLShader(const char* shaderName);
    GLShader(const char* vertShaderPath, const char* fragShaderPath);
    ~GLShader();
    
    bool IsGood();
    
    GLuint GetProgram() { return mProgram; }
    
    void Activate();
    
    void SetUniformMatrix4(const char* name, const Matrix4& mat);
    
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
