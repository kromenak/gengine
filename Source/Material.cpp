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

Matrix4 Material::sCurrentViewMatrix;
Matrix4 Material::sCurrentProjMatrix;

float Material::sAlphaTestValue = 0.0f;

void Material::SetViewMatrix(const Matrix4& viewMatrix)
{
	sCurrentViewMatrix = viewMatrix;
}

void Material::SetProjMatrix(const Matrix4& projMatrix)
{
	sCurrentProjMatrix = projMatrix;
}

void Material::UseAlphaTest(bool use)
{
	sAlphaTestValue = use ? 0.1f : 0.0f;
}

Material::Material() : mShader(sDefaultShader)
{
    
}

Material::Material(Shader* shader) : mShader(shader)
{
	
}

bool Material::operator==(const Material& other) const
{
    return mShader == other.mShader
	&& mDiffuseTexture == other.mDiffuseTexture
	&& mColor == other.mColor;
}

void Material::Activate()
{
	// Activate the shader.
    mShader->Activate();
	
	// Set view/proj matrix.
	mShader->SetUniformMatrix4("uViewProj", sCurrentProjMatrix * sCurrentViewMatrix);
	
	// Set alpha test value.
	mShader->SetUniformFloat("uAlphaTest", sAlphaTestValue);
	
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

bool Material::IsTranslucent()
{
	if(mDiffuseTexture != nullptr)
	{
		return mDiffuseTexture->GetRenderType() == Texture::RenderType::Translucent;
	}
	return false;
}
