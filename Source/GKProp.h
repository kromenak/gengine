//
// GKProp.h
//
// Clark Kromenaker
//
// Description
//
#pragma once
#include "GKObject.h"

class GAS;

class GKProp : public GKObject
{
public:
	GKProp();
	
	void SetGas(GAS* gas);
	
protected:
	
private:
};
