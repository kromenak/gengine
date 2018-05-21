//
// GLShader.cpp
//
// Clark Kromenaker
//
#include "GLShader.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include "Matrix4.h"

GLShader::GLShader(const char* vertShaderPath, const char* fragShaderPath)
{
    // Load vertex and fragment shaders, and compile them.
    GLuint vertexShader = LoadAndCompileShaderFromFile(vertShaderPath, GL_VERTEX_SHADER);
    GLuint fragmentShader = LoadAndCompileShaderFromFile(fragShaderPath, GL_FRAGMENT_SHADER);
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
    
    // Query for attribute locations.
    //GLint posAttribIndex = glGetAttribLocation(mProgram, "v2p");
    //std::cout << posAttribIndex << std::endl;
    
    // Detach shaders after a successful link.
    glDetachShader(mProgram, vertexShader);
    glDetachShader(mProgram, fragmentShader);
}

GLShader::~GLShader()
{
    if(mProgram != GL_NONE)
    {
        glDeleteProgram(mProgram);
    }
}

bool GLShader::IsGood()
{
    return !mError;
}

void GLShader::Activate()
{
    glUseProgram(mProgram);
}

void GLShader::SetUniformMatrix4(const char* name, const Matrix4& mat)
{
    GLuint loc = glGetUniformLocation(mProgram, name);
    glUniformMatrix4fv(loc, 1, GL_FALSE, mat.GetFloatPtr());
}

GLuint GLShader::LoadAndCompileShaderFromFile(const char* filePath, GLuint shaderType)
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

bool GLShader::IsShaderCompiled(GLuint shader)
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

bool GLShader::IsProgramLinked(GLuint program)
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
