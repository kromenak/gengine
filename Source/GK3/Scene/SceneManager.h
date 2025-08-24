//
// Clark Kromenaker
//
// Manages loading and unloading scenes.
//
#pragma once
#include <functional>
#include <string>
#include <vector>

#include "Scene.h"

class Actor;

class SceneManager
{
public:
    void Shutdown();

    void Update(float deltaTime);
    void UpdateLoading();

    void LoadScene(const std::string& name, const std::function<void()>& callback = nullptr, bool callEnterOnSceneLoad = true);
    void UnloadScene(const std::function<void()>& callback = nullptr);
    bool IsSceneLoading() const { return mSceneLoading; }

    Scene* GetScene() const { return mScene; }

    void AddActor(Actor* actor) { mActors.push_back(actor); }
    const std::vector<Actor*>& GetActors() const { return mActors; }

private:
    // The active scene. Curretly, there can be only one at a time.
    Scene* mScene = nullptr;

    // A scene that's been requested to load. If empty, no pending scene change.
    // Scene loads happen at the end of a frame, to avoid a scene change mid-frame.
    std::string mSceneToLoad;

    // If set, we explicitly want to unload the current scene without loading a new scene.
    bool mUnloadScene = false;

    // If true, we are actively loading a scene.
    bool mSceneLoading = false;

    // Callbacks to execute when scene load/unload completes.
    std::function<void()> mSceneLoadedCallback = nullptr;
    std::function<void()> mSceneUnloadedCallback = nullptr;

    // If true, the scene "Enter" function will be called after load.
    // We usually want to do this, unless we are loading a save game.
    bool mCallEnterOnSceneLoad = true;

    // A list of all actors that currently exist in the game.
    std::vector<Actor*> mActors;

    void LoadSceneInternal();
    void UnloadSceneInternal();

    void DeleteDestroyedActors();
};

extern SceneManager gSceneManager;