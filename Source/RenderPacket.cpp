//
// RenderPacket.cpp
//
// Clark Kromenaker
//
#include "RenderPacket.h"

#include "Mesh.h"

void RenderPacket::Render()
{
    material.SetWorldTransformMatrix(worldTransform);
    material.Activate();
    mesh->Render();
}
