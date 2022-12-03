//
// Clark Kromenaker
//
// A texture that can be rendered to. When activated, render commands
// will go to the texture instead of the screen.
//
// After rendering to the texture, the texture can be used as an
// input to other rendering operations that ultimately go to the screen.
//
// In OpenGL, this is implemented using "Framebuffer Objects."
//
#pragma once
#include <GL/glew.h>

#include "Texture.h"

class RenderTexture
{
public:
	RenderTexture(int width, int height);
	~RenderTexture();
	
	void Activate();
	
	Texture& GetTexture() { return mRenderTexture; }
	
private:
	GLuint mFboId = GL_NONE;
	Texture mRenderTexture;
	
	GLuint mRenderTextureId = GL_NONE;
};
