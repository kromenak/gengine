#include "SceneLayer.h"

#include "Scene.h"
#include "SoundtrackPlayer.h"
#include "StringUtil.h"

SceneLayer::SceneLayer(Scene* scene) : Layer("SceneLayer"),
    mScene(scene)
{
    
}

void SceneLayer::OnEnter(Layer* fromLayer)
{
    mScene->SetPaused(false);
}

void SceneLayer::OnExit(Layer* toLayer)
{
    if(mScene->IsLoaded())
    {
        mScene->SetPaused(true);

        // Ambient audio (aka music) is sometimes NOT disabled when pushing on a layer.
        // For example, when inventory is showing, scene music continues to play.
        //TODO: Probably doesn't make sense to keep this here - OnEnter, other layers should maybe disable soundtrack if desired?
        mScene->GetSoundtrackPlayer()->SetEnabled(toLayer != nullptr && StringUtil::EqualsIgnoreCase(toLayer->GetName(), "InventoryLayer"));
    }
}
