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
//#include <OpenGL/gl.h>

class Color32;
class Matrix4;
class Vector3;
class Vector4;

class Shader
{
public:
    Shader(const char* vertShaderPath, const char* fragShaderPath);
    ~Shader();
    
    void Activate();
	
	void SetUniformInt(const char* name, int value);
	void SetUniformFloat(const char* name, float value);
	
    void SetUniformVector3(const char* name, const Vector3& vector);
	void SetUniformVector4(const char* name, const Vector4& vector);
    
    void SetUniformMatrix4(const char* name, const Matrix4& mat);
    
    void SetUniformColor(const char* name, const Color32& color);
    
    bool IsGood() const { return mProgram != GL_NONE; }
    
private:
    // Handle to the compiled and linked GL shader program.
    GLuint mProgram = GL_NONE;
    
    GLuint LoadAndCompileShaderFromFile(const char* filePath, GLuint shaderType);
    
    bool IsShaderCompiled(GLuint shader);
    bool IsProgramLinked(GLuint program);
};
