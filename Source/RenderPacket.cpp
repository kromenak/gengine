//
// RenderPacket.cpp
//
// Clark Kromenaker
//
#include "RenderPacket.h"

#include "Material.h"
#include "Mesh.h"
#include "Submesh.h"

bool RenderPacket::IsTransparent()
{
	for(int i = 0; i < materialsCount; i++)
	{
		if(materials[i].IsTransparent()) { return true; }
	}
	return false;
}

void RenderPacket::Render()
{
	const std::vector<Submesh*>& submeshes = mesh->GetSubmeshes();
	for(int i = 0; i < submeshes.size(); i++)
	{
		if(materials != nullptr)
		{
			materials[i].SetWorldTransformMatrix(worldTransform);
			materials[i].Activate();
		}
		submeshes[i]->Render();
	}
	
    //material.SetWorldTransformMatrix(worldTransform);
    //material.Activate();
    //mesh->Render();
}
