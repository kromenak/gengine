//
// UIImage.cpp
//
// Clark Kromenaker
//
#include "UIImage.h"

#include "Actor.h"
#include "Camera.h"
#include "Debug.h"
#include "Mesh.h"
#include "Texture.h"

extern Mesh* uiQuad;

//HACK: Need to revert quad UVs after rendering with repeat.
static float quad_uvs[] = {
	0.0f, 0.0f,		// upper-left
	1.0f, 0.0f,		// upper-right
	1.0f, 1.0f,		// lower-right
	0.0f, 1.0f		// lower-left
};

TYPE_DEF_CHILD(UIWidget, UIImage);

UIImage::UIImage(Actor* owner) : UIWidget(owner)
{
    
}

void UIImage::Render()
{
	// We need a texture to render (and calculate repeats for tiled rendering).
	Texture* texture = mMaterial.GetDiffuseTexture();
	if(texture == nullptr) { return; }
	
	// Prep the material.
	mMaterial.SetWorldTransformMatrix(GetWorldTransformWithSizeForRendering());
	mMaterial.Activate();
	
	// Render based on desired render mode.
	switch(mRenderMode)
	{
		case RenderMode::Normal:
		{
			uiQuad->Render();
			break;
		}
		case RenderMode::Tiled:
		{
			// Determine how many repeats are needed.
			Vector2 size = mRectTransform->GetSize();
			float repeatX = size.GetX() / texture->GetWidth();
			float repeatY = size.GetY() / texture->GetHeight();
			
			// Adjust UVs for repeating textures.
			float repeat_uvs[] = {
				0.0f,    0.0f,		// upper-left
				repeatX, 0.0f,		// upper-right
				repeatX, repeatY,	// lower-right
				0.0f,    repeatY	// lower-left
			};
			uiQuad->GetSubmesh(0)->SetUV1(repeat_uvs);
			
			// Render.
			uiQuad->Render();
			
			// Revert to "normal" UVs.
			uiQuad->GetSubmesh(0)->SetUV1(quad_uvs);
		}
		//TODO: Nine-slice?
	}
}

void UIImage::SetTexture(Texture* texture)
{
	mMaterial.SetDiffuseTexture(texture);
	
}

void UIImage::SetSizeToTextureSize()
{
	// Need a texture to do this!
	Texture* texture = mMaterial.GetDiffuseTexture();
	if(texture == nullptr) { return; }
	
	// Set size from texture.
	mRectTransform->SetSizeDelta(texture->GetWidth(), texture->GetHeight());
}
