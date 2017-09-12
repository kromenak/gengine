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
#include "Matrix4.h"

GLfloat triangle_vertices[] = {
    0.0f,  0.5f,  0.0f,
    0.5f, -0.5f,  0.0f,
    -0.5f, -0.5f,  0.0f
};

GLfloat cube_vertices[] = {
    // front
    -1.0, -1.0,  1.0,
    1.0, -1.0,  1.0,
    1.0,  1.0,  1.0,
    -1.0,  1.0,  1.0,
    // back
    -1.0, -1.0, -1.0,
    1.0, -1.0, -1.0,
    1.0,  1.0, -1.0,
    -1.0,  1.0, -1.0
};

GLfloat cube_colors[] = {
    // front colors
    1.0, 0.0, 0.0,
    0.0, 1.0, 0.0,
    0.0, 0.0, 1.0,
    1.0, 1.0, 1.0,
    // back colors
    1.0, 0.0, 0.0,
    0.0, 1.0, 0.0,
    0.0, 0.0, 1.0,
    1.0, 1.0, 1.0,
};

GLushort cube_elements[] = {
    // front
    0, 1, 2,
    2, 3, 0,
    // top
    1, 5, 6,
    6, 2, 1,
    // back
    7, 6, 5,
    5, 4, 7,
    // bottom
    4, 0, 3,
    3, 7, 4,
    // left
    4, 5, 1,
    1, 0, 4,
    // right
    3, 2, 6,
    6, 7, 3,
};

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
    
    // Clear any GLEW error.
    glGetError();
    
    // Initialize frame buffer.
    glEnable(GL_BLEND);
    glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    
    mShader = new GLShader("Assets/Tut.vert", "Assets/Tut.frag");
    if(!mShader->IsGood())
    {
        return false;
    }
    mVertArray = new GLVertexArray(triangle_vertices, 9);
    
    // Init succeeded!
    return true;
}

void SDLRenderer::Shutdown()
{
    delete mVertArray;
    delete mShader;
    
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
    mShader->Activate();
    
    mVertArray->Draw();
    
    /*
    glUseProgram(mBasicMeshProgram);
    
    Matrix4 viewProj;
    GLuint view = glGetUniformLocation(mBasicMeshProgram, "uViewProj");
    glUniformMatrix4fv(view, 1, GL_FALSE, viewProj.GetFloatPtr());
    
    Matrix4 worldTransform;
    GLuint world = glGetUniformLocation(mBasicMeshProgram, "uWorldTransform");
    glUniformMatrix4fv(world, 1, GL_FALSE, worldTransform.GetFloatPtr());
    */
    
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
