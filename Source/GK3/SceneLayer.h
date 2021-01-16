//
// SceneLayer.h
//
// Clark Kromenaker
//
// A layer for the Scene.
//
// Primarily responsible for resuming/suspending scene layer behavior.
//
#pragma once
#include "LayerManager.h"

class Scene;

class SceneLayer : public Layer
{
public:
    SceneLayer(Scene* scene);
    
protected:
    void OnEnter() override;
    void OnExit() override;
    
private:
    Scene* mScene = nullptr;
};
