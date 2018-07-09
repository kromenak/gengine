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
    mShader->Activate();
    
    if(mDiffuseTexture != nullptr)
    {
        mDiffuseTexture->Activate();
    }
}

void Material::SetWorldTransformMatrix(const Matrix4& mat)
{
    mShader->SetUniformMatrix4("uWorldTransform", mat);
}
