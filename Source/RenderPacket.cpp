//
// RenderPacket.cpp
//
// Clark Kromenaker
//
#include "RenderPacket.h"

void RenderPacket::Render()
{
    material.SetWorldTransformMatrix(worldTransform);
    material.Activate();
    mesh->Render();
}
