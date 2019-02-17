//
// Material.cpp
//
// Clark Kromenaker
//
#include "Material.h"

#include "Shader.h"
#include "Texture.h"

// This is set to a default shader during Renderer init.
Shader* Material::sDefaultShader = nullptr;

Material::Material() : mShader(sDefaultShader)
{
    
}

bool Material::operator==(const Material& other) const
{
    return mShader == other.mShader && mDiffuseTexture == other.mDiffuseTexture;
}

void Material::Activate()
{
	// Activate the shader.
    mShader->Activate();
	
	// Set the uniform color for the shader.
	SetActiveColor(mColor);
	
	// Activate the diffuse texture, if any.
    if(mDiffuseTexture != nullptr)
    {
        mDiffuseTexture->Activate();
    }
	else
	{
		Texture::Deactivate();
	}
}

void Material::SetWorldTransformMatrix(const Matrix4& matrix)
{
    mShader->SetUniformMatrix4("uWorldTransform", matrix);
}

void Material::SetActiveColor(const Color32& color)
{
	mShader->SetUniformVector4("uColor", color);
}

bool Material::IsTransparent()
{
	if(mDiffuseTexture != nullptr)
	{
		return mDiffuseTexture->HasAlpha();
	}
	return false;
}
