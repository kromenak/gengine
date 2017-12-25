//
// CameraComponent.h
//
// Clark Kromenaker
//
#pragma once
#include "Component.h"
#include "Matrix4.h"

class CameraComponent : public Component
{
public:
    CameraComponent(Actor* owner);
    
    void Render();
    
    Matrix4 GetLookAtMatrix();
    Matrix4 GetProjectionMatrix();
    
private:
};
