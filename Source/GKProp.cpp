//
// GKProp.cpp
//
// Clark Kromenaker
//
#include "GKProp.h"

#include "GasPlayer.h"
#include "MeshRenderer.h"
#include "VertexAnimationPlayer.h"

GKProp::GKProp() : GKObject(true)
{
	
}

void GKProp::SetGas(GAS* gas)
{
	mGasPlayer->SetGas(gas);
}
