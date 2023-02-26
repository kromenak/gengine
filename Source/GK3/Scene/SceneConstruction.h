//
// Clark Kromenaker
//
// Support & helpers for a scene's "construction mode".
// Construction mode is enabled with the TAB key.
//
#pragma once
#include <vector>

class Actor;
class Model;
class Scene;
class SceneData;

class SceneConstruction
{
public:
    void Init(Scene* scene, SceneData* sceneData);

    void Render();

    void SetShowCameraBounds(bool show) { mShowCameraBounds = show; }
    bool GetShowCameraBounds() const { return mShowCameraBounds; }

    void SetShowWalkerBoundary(bool show);
    bool GetShowWalkerBoundary() const;

private:
    Scene* mScene = nullptr;
    SceneData* mSceneData = nullptr;

    // Camera bounds visualization support.
    bool mShowCameraBounds = false;
    std::vector<Model*> mCameraBoundsModels;

    // Walker boundary visualization.
    Actor* mWalkerBoundaryActor = nullptr;
};