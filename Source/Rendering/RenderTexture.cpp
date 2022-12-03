#include "RenderTexture.h"

#include <iostream>

RenderTexture::RenderTexture(int width, int height) :
	mRenderTexture(width, height, Color32::Black)
{
	// Create framebuffer object to do rendering through.
	glGenFramebuffers(1, &mFboId);
	
	// Bind our framebuffer object, so we can attach to it.
	glBindFramebuffer(GL_FRAMEBUFFER, mFboId);
	
	// Configure generated texture as render color target for the FBO.
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mRenderTexture.mTextureId, 0);
	
	// Set outputs for fragment data in shaders.
	// Basically, we want our fragment shader to output to GL_COLOR_ATTACHMENT0.
	// GL_COLOR_ATTACHMENT0 is where our texture is attached.
	GLenum drawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, drawBuffers);
	
	// Make sure FBO is set up correctly.
	GLenum result = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if(result != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "Problem creating render texture frame buffer!" << std::endl;
	}
	
	// Unbind frame buffer until we need it again.
	glBindFramebuffer(GL_FRAMEBUFFER, GL_NONE);
}

RenderTexture::~RenderTexture()
{
	glDeleteFramebuffers(1, &mFboId);
	glDeleteTextures(1, &mRenderTextureId);
}

void RenderTexture::Activate()
{
	glBindFramebuffer(GL_FRAMEBUFFER, mFboId);
	glViewport(0, 0, 256, 256); // Render on the whole framebuffer, complete from the lower left corner to the upper right
}
