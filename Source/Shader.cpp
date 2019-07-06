//
// Shader.cpp
//
// Clark Kromenaker
//
#include "Shader.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include "Color32.h"
#include "Matrix4.h"
#include "Vector3.h"

Shader::Shader(const char* vertShaderPath, const char* fragShaderPath)
{
    // Load vertex and fragment shaders, and compile them.
    GLuint vertexShader = LoadAndCompileShaderFromFile(vertShaderPath, GL_VERTEX_SHADER);
    GLuint fragmentShader = LoadAndCompileShaderFromFile(fragShaderPath, GL_FRAGMENT_SHADER);
    
    // If either shader could not be compiled successfully, fail with an error.
    if(!IsShaderCompiled(vertexShader) || !IsShaderCompiled(fragmentShader))
    {
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        mError = true;
        return;
    }
    
    // Assemble shader program.
    mProgram = glCreateProgram();
    glAttachShader(mProgram, vertexShader);
    glAttachShader(mProgram, fragmentShader);
    
    // Link the shader program.
    glLinkProgram(mProgram);
    if(!IsProgramLinked(mProgram))
    {
        glDeleteProgram(mProgram);
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        mError = true;
        return;
    }
    
    // Detach shaders after a successful link.
    glDetachShader(mProgram, vertexShader);
    glDetachShader(mProgram, fragmentShader);
}

Shader::~Shader()
{
    glDeleteProgram(mProgram);
}

void Shader::Activate()
{
    glUseProgram(mProgram);
}

GLuint Shader::GetAttributeLocation(const char* name) const
{
    return glGetAttribLocation(mProgram, name);
}

void Shader::SetUniformInt(const char* name, int value)
{
	GLuint loc = glGetUniformLocation(mProgram, name);
	glUniform1i(loc, value);
}

void Shader::SetUniformVector3(const char* name, const Vector3& vector)
{
    GLuint vecLoc = glGetUniformLocation(mProgram, name);
    glUniform3f(vecLoc, vector.GetX(), vector.GetY(), vector.GetZ());
}

void Shader::SetUniformVector4(const char *name, const Vector4& vector)
{
	GLuint vecLoc = glGetUniformLocation(mProgram, name);
	glUniform4f(vecLoc, vector.GetX(), vector.GetY(), vector.GetZ(), vector.GetW());
}

void Shader::SetUniformVector4(const char* name, const Color32& color)
{
	GLuint vecLoc = glGetUniformLocation(mProgram, name);
	glUniform4f(vecLoc, color.GetR() / 255.0f, color.GetG() / 255.0f, color.GetB() / 255.0f, color.GetA() / 255.0f);
}

void Shader::SetUniformMatrix4(const char* name, const Matrix4& mat)
{
    GLuint loc = glGetUniformLocation(mProgram, name);
    glUniformMatrix4fv(loc, 1, GL_FALSE, mat.GetFloatPtr());
}

GLuint Shader::LoadAndCompileShaderFromFile(const char* filePath, GLuint shaderType)
{
    // Open the file, but freak out if not valid.
    std::ifstream file(filePath);
    if(!file.good())
    {
        std::cout << "Couldn't open shader file for loading: " << filePath << std::endl;
        return GL_NONE;
    }
    
    // Read the file contents into a char buffer.
    // Important to read to intermediate std::string first!
    // Doesn't read correctly without that bit.
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string fileContentsStr = buffer.str();
    const char* fileContents = fileContentsStr.c_str();
    
    // Create shader, load file contents into it, and compile it.
    GLuint shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, &fileContents, nullptr);
    glCompileShader(shader);
    return shader;
}

bool Shader::IsShaderCompiled(GLuint shader)
{
    // Ask GL whether compile succeeded for this shader.
    GLint compileSucceeded = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compileSucceeded);
    
    // If not, we'll output the error log and fail.
    if(compileSucceeded == GL_FALSE)
    {
        GLint errorLength = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &errorLength);
        
        GLchar* errorLog = new GLchar[errorLength];
        glGetShaderInfoLog(shader, errorLength, &errorLength, errorLog);
        
        std::cout << "Error compiling shader: " << errorLog << std::endl;
        delete[] errorLog;
        return false;
    }
    
    // GL reports the compilation was successful!
    return true;
}

bool Shader::IsProgramLinked(GLuint program)
{
    // Ask GL whether link succeeded for this program.
    GLint linkSucceeded = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &linkSucceeded);
    
    // If not, we'll output the error log and fail.
    if(linkSucceeded == GL_FALSE)
    {
        GLint errorLength = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &errorLength);
        
        GLchar* errorLog = new GLchar[errorLength];
        glGetProgramInfoLog(program, errorLength, &errorLength, errorLog);
        
        std::cout << "Error linking shader program: " << errorLog << std::endl;
        delete[] errorLog;
        return false;
    }
    
    // GL reports the linking was successful!
    return true;
}
