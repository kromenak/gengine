//
// Clark Kromenaker
//
// A layer for the Scene.
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
    void OnEnter(Layer* fromLayer) override;
    void OnExit(Layer* toLayer) override;
    
private:
    Scene* mScene = nullptr;
};
