//
// Shader.h
//
// Clark Kromenaker
//
// A compiled and linked shader program.
//
#pragma once
#include <string>
#include <GL/glew.h>

class Vector3;
class Matrix4;

class Shader
{
public:
    Shader(const char* vertShaderPath, const char* fragShaderPath);
    ~Shader();
    
    void Activate();
    
    GLuint GetAttributeLocation(const char* name);
    
    void SetUniformVector3(const char* name, const Vector3& vector);
    void SetUniformMatrix4(const char* name, const Matrix4& mat);
    
    bool IsGood() { return !mError; }
    
private:
    // Handle to the compiled and linked GL shader program.
    GLuint mProgram = GL_NONE;
    
    // Did we encounter an error during compile/linking?
    bool mError = false;
    
    GLuint LoadAndCompileShaderFromFile(const char* filePath, GLuint shaderType);
    bool IsShaderCompiled(GLuint shader);
    bool IsProgramLinked(GLuint program);
};
