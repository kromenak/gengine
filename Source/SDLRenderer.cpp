//
//  SDLRenderer.cpp
//  GEngine
//
//  Created by Clark Kromenaker on 7/22/17.
//

#include "SDLRenderer.h"
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

bool SDLRenderer::Initialize()
{
    // Init video subsystem.
    if(SDL_InitSubSystem(SDL_INIT_VIDEO) != 0)
    {
        return false;
    }
    
    // Tell SDL we want to use OpenGL 3.3
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    
    // Request some GL parameters, just in case
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    
    // Create a window.
    mWindow = SDL_CreateWindow("GK3", 100, 100, 1024, 768, SDL_WINDOW_OPENGL);
    if(!mWindow) { return false; }
    
    // Create OpenGL context.
    mContext = SDL_GL_CreateContext(mWindow);
    
    // Initialize GLEW.
    glewExperimental = GL_TRUE;
    if(glewInit() != GLEW_OK)
    {
        SDL_Log("Failed to initialize GLEW.");
        return false;
    }
    
    // Clear any Glew error.
    glGetError();
    
    // Initialize frame buffer.
    glEnable(GL_BLEND);
    glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    
    // Compile default shader program.
    GLuint vertexShader = LoadAndCompileShaderFromFile("Assets/BasicMesh.vert", GL_VERTEX_SHADER);
    GLuint fragmentShader = LoadAndCompileShaderFromFile("Assets/BasicMesh.frag", GL_FRAGMENT_SHADER);
    if(!IsShaderCompiled(vertexShader) || !IsShaderCompiled(fragmentShader))
    {
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        return false;
    }
    
    // Assemble shader program.
    mBasicMeshProgram = glCreateProgram();
    glAttachShader(mBasicMeshProgram, vertexShader);
    glAttachShader(mBasicMeshProgram, fragmentShader);
    
    glBindFragDataLocation(mBasicMeshProgram, 0, "outColor");
    glLinkProgram(mBasicMeshProgram);
    if(!IsProgramLinked(mBasicMeshProgram))
    {
        glDeleteProgram(mBasicMeshProgram);
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        return false;
    }
    
    // Detach shaders after a successful link.
    glDetachShader(mBasicMeshProgram, vertexShader);
    glDetachShader(mBasicMeshProgram, fragmentShader);
    
    // Init succeeded!
    return true;
}

void SDLRenderer::Shutdown()
{
    glDeleteProgram(mBasicMeshProgram);
    
    SDL_GL_DeleteContext(mContext);
    SDL_DestroyWindow(mWindow);
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
}

void SDLRenderer::Clear()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void SDLRenderer::Render()
{
    //TODO: Draw stuff...
}

void SDLRenderer::Present()
{
    SDL_GL_SwapWindow(mWindow);
}

GLuint SDLRenderer::LoadAndCompileShaderFromFile(const char* filePath, GLuint shaderType)
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

bool SDLRenderer::IsShaderCompiled(GLuint shader)
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

bool SDLRenderer::IsProgramLinked(GLuint program)
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
