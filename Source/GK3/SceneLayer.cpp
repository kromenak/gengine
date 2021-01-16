//
// SceneLayer.cpp
//
// Clark Kromenaker
//
#include "SceneLayer.h"

#include "Scene.h"

SceneLayer::SceneLayer(Scene* scene) : Layer("SceneLayer"),
    mScene(scene)
{
    
}

void SceneLayer::OnEnter()
{
    mScene->SetPaused(false);
}

void SceneLayer::OnExit()
{
    mScene->SetPaused(true);
}
