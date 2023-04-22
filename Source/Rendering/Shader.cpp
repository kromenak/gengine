#include "Shader.h"

#include <iostream>
#include <fstream>
#include <sstream>

#include "Color32.h"
#include "Matrix4.h"
#include "ThreadUtil.h"
#include "Vector3.h"
#include "VertexDefinition.h"

Shader::Shader(const char* vertShaderPath, const char* fragShaderPath) : Asset(vertShaderPath)
{
    // Load vertex and fragment shaders, and compile them.
    GLuint vertexShader = LoadAndCompileShaderFromFile(vertShaderPath, GL_VERTEX_SHADER);
    GLuint fragmentShader = LoadAndCompileShaderFromFile(fragShaderPath, GL_FRAGMENT_SHADER);

    // If either shader could not be compiled successfully, fail with an error.
    if(!IsShaderCompiled(vertexShader) || !IsShaderCompiled(fragmentShader))
    {
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        return;
    }

    // Assemble shader program.
    mProgram = glCreateProgram();
    glAttachShader(mProgram, vertexShader);
    glAttachShader(mProgram, fragmentShader);

    // Bind shader attribute names to attribute indexes.
    int semanticCount = static_cast<int>(VertexAttribute::Semantic::SemanticCount);
    for(int i = 0; i < semanticCount; ++i)
    {
        glBindAttribLocation(mProgram, i, gAttributeNames[i]);
    }

    // Link the shader program.
    glLinkProgram(mProgram);
    if(!IsProgramLinked(mProgram))
    {
        glDeleteProgram(mProgram);
        mProgram = GL_NONE;

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        return;
    }

    // Detach shaders after a successful link.
    glDetachShader(mProgram, vertexShader);
    glDetachShader(mProgram, fragmentShader);

    // After shader program is compiled and linked, it's possible to query the program
    // to determine the uniforms that exist in the program.

    // This *may* be useful in the future so that a material knows what uniforms exist.
    // But for now, we are assuming that the material has explicitly defined values for all uniforms.
    RefreshUniforms();
}

Shader::~Shader()
{
    glDeleteProgram(mProgram);
}

void Shader::Activate()
{
    if(mProgram != GL_NONE)
    {
        glUseProgram(mProgram);
    }
}

void Shader::SetUniformInt(const char* name, int value)
{
    if(mProgram != GL_NONE)
    {
        GLuint loc = glGetUniformLocation(mProgram, name);
        if(loc >= 0)
        {
            glUniform1i(loc, value);
        }
    }
}

void Shader::SetUniformFloat(const char* name, float value)
{
    if(mProgram != GL_NONE)
    {
        GLuint loc = glGetUniformLocation(mProgram, name);
        if(loc >= 0)
        {
            glUniform1f(loc, value);
        }
    }
}

void Shader::SetUniformVector3(const char* name, const Vector3& vector)
{
    if(mProgram != GL_NONE)
    {
        GLuint loc = glGetUniformLocation(mProgram, name);
        if(loc >= 0)
        {
            glUniform3f(loc, vector.x, vector.y, vector.z);
        }
    }
}

void Shader::SetUniformVector4(const char *name, const Vector4& vector)
{
    if(mProgram != GL_NONE)
    {
        GLuint loc = glGetUniformLocation(mProgram, name);
        if(loc >= 0)
        {
            glUniform4f(loc, vector.x, vector.y, vector.z, vector.w);
        }
    }
}

void Shader::SetUniformMatrix4(const char* name, const Matrix4& mat)
{
    if(mProgram != GL_NONE)
    {
        GLuint loc = glGetUniformLocation(mProgram, name);
        if(loc >= 0)
        {
            glUniformMatrix4fv(loc, 1, GL_FALSE, mat);
        }
    }
}

void Shader::SetUniformColor(const char* name, const Color32& color)
{
    if(mProgram != GL_NONE)
    {
        GLuint loc = glGetUniformLocation(mProgram, name);
        if(loc >= 0)
        {
            glUniform4f(loc, color.GetR() / 255.0f, color.GetG() / 255.0f, color.GetB() / 255.0f, color.GetA() / 255.0f);
        }
    }
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

void Shader::RefreshUniforms()
{
    // Because this code may modify shader uniforms, we must first activate the shader.
    Activate();

    // These vars are reused when obtaining info about uniforms in loop below.
    const GLsizei kMaxUniformNameLength = 32;
    GLchar uniformNameBuffer[kMaxUniformNameLength];
    GLsizei uniformNameLength = 0;
    GLsizei uniformSize = 0;
    GLenum uniformType = GL_NONE;

    // Determine count of uniforms in this shader program.
    GLint uniformCount = 0;
    glGetProgramiv(mProgram, GL_ACTIVE_UNIFORMS, &uniformCount);

    // Iterate uniforms and process each one.
    int textureUnitCounter = 0;
    for(GLuint i = 0; i < uniformCount; ++i)
    {
        // Grab uniform i.
        glGetActiveUniform(mProgram, i, kMaxUniformNameLength, &uniformNameLength, &uniformSize, &uniformType, uniformNameBuffer);
        
        // If returned name length is 0, that means the uniform is not valid (compile/link failed?).
        if(uniformNameLength <= 0) { continue; }
        
        // We only want to track user-defined uniforms.
        // We want to ignore built-in OpenGL uniforms, which have "gl_" prefix.
        // We want to ignore built-in G-Engine uniforms, which have a "g" prefix.
        // So really, we can just ignore any uniform with a "g" prefix to cover both scenarios!
        if(uniformNameBuffer[0] == 'g') { continue; }
        //printf("Uniform %d, Type %u, Name %s\n", i, uniformType, uniformNameBuffer);

        // For texture samplers, you must tell OpenGL which "texture unit" to use.
        // If the shader only uses one texture sampler, this works automatically.
        // But you must manually specify the unit if more than one texture is used.
        if(uniformType == GL_SAMPLER_2D)
        {
            SetUniformInt(uniformNameBuffer, textureUnitCounter);
            ++textureUnitCounter;
        }

        /*
        // Convert GLenum type to an actual enum type.
        UniformType type = UniformType::Unknown;
        switch(uniformType)
        {
        case GL_FLOAT:
            type = UniformType::Float;
            break;
        case GL_INT:
            type = UniformType::Int;
            break;
        case GL_UNSIGNED_INT:
            type = UniformType::Uint;
            break;
        case GL_BOOL:
            type = UniformType::Bool;
            break;
            
        case GL_FLOAT_VEC2:
            type = UniformType::Vector2;
            break;
        case GL_FLOAT_VEC3:
            type = UniformType::Vector3;
            break;
        case GL_FLOAT_VEC4:
            type = UniformType::Vector4;
            break;
            
        case GL_FLOAT_MAT2:
            type = UniformType::Matrix2;
            break;
        case GL_FLOAT_MAT3:
            type = UniformType::Matrix3;
            break;
        case GL_FLOAT_MAT4:
            type = UniformType::Matrix4;
            break;
            
        case GL_SAMPLER_2D:
            type = UniformType::Texture2D;
            break;
        case GL_SAMPLER_CUBE:
            type = UniformType::TextureCube;
            break;
            
        default:
            std::cout << "Unknown uniform type in shader: " << uniformType << std::endl;
            break;
        }

        // Create and save uniform info.
        if(type != UniformType::Unknown)
        {
            Uniform uniform;
            uniform.type = type;
            uniform.name = std::string(uniformNameBuffer);
            mUniforms.push_back(uniform);
        }
        */
    }
}
