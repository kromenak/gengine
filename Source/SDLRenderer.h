//
//  SDLRenderer.h
//  GEngine
//
//  Created by Clark Kromenaker on 7/22/17.
//
#pragma once

#include "SDL/SDL.h"
#include <GL/glew.h>
#include "GLVertexArray.h"
#include "GLShader.h"

class SDLRenderer
{
private:
    // Handle for the window object (contains the game).
    SDL_Window* mWindow = nullptr;
    
    // Context for rendering in OpenGL.
    SDL_GLContext mContext;
    
    // Compiled default shader program.
    GLuint mBasicMeshProgram = GL_NONE;
    
    GLShader* mShader = nullptr;
    GLVertexArray* mVertArray = nullptr;
    
public:
    bool Initialize();
    void Shutdown();
    
    void Clear();
    void Render();
    void Present();
    
private:
    GLuint LoadAndCompileShaderFromFile(const char* filePath, GLuint shaderType);
    bool IsShaderCompiled(GLuint shader);
    bool IsProgramLinked(GLuint program);
};
