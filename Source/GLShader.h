//
// GLShader.h
//
// Clark Kromenaker
//
// A compiled and linked GLSL shader program.
//
#pragma once
#include <string>
#include <GL/glew.h>

class Matrix4;

class GLShader
{
public:
    GLShader(const char* vertShaderPath, const char* fragShaderPath);
    ~GLShader();
    
    void Activate();
    
    void SetUniformMatrix4(const char* name, const Matrix4& mat);
    
    bool IsGood() { return !mError; }
    GLuint GetProgram() { return mProgram; }
    
private:
    // Handle to the compiled and linked GL shader program.
    GLuint mProgram = GL_NONE;
    
    // Did we encounter an error during compile/linking?
    bool mError = false;
    
    GLuint LoadAndCompileShaderFromFile(const char* filePath, GLuint shaderType);
    bool IsShaderCompiled(GLuint shader);
    bool IsProgramLinked(GLuint program);
};
