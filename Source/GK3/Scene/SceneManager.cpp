#include "SceneManager.h"

#include "Actor.h"
#include "AssetManager.h"
#include "Loader.h"
#include "Profiler.h"

SceneManager gSceneManager;

void SceneManager::Shutdown()
{
    // Unload any loaded scene.
    // This will delete all actors local to the scene.
    UnloadSceneInternal();

    // Delete all actors (even the "don't destroy on load" ones).
    for(auto& actor : mActors)
    {
        delete actor;
    }
    mActors.clear();
}

void SceneManager::Update(float deltaTime)
{
    if(mSceneLoading) { return; }

    if(mScene != nullptr)
    {
        mScene->Update(deltaTime);
    }

    // Update actors, but *don't* update actors that are added when updating other actors!
    // To guard against this, get size first and only update to that point.
    size_t size = mActors.size();
    for(size_t i = 0; i < size; ++i)
    {
        mActors[i]->Update(deltaTime);
    }

    // Do a late update step on all actors.
    // Why is this needed? In some cases, an Actor must update after some other actor has updated.
    // An easy way to enable this is to do another update pass after the original update pass.
    for(size_t i = 0; i < size; ++i)
    {
        mActors[i]->LateUpdate(deltaTime);
    }

    // Delete any destroyed actors.
    DeleteDestroyedActors();
}

void SceneManager::UpdateLoading()
{
    // If neither loading or unloading, we can early out.
    if(mSceneToLoad.empty() && !mUnloadScene) { return; }

    // Wait until loader is idle before unloading or loading any scene.
    if(Loader::IsLoading()) { return; }

    // FROM HERE: We want to unload the current scene and (possibly) load a new scene!

    // We either want to unload the scene only, or unload the current scene and load a new scene.
    // In both cases...we need to unload first!
    UnloadSceneInternal();
    mUnloadScene = false; // did it

    // Load the new scene if needed.
    if(!mSceneToLoad.empty())
    {
        LoadSceneInternal();
        mSceneToLoad.clear(); // did it
    }
}

void SceneManager::LoadScene(const std::string& name, const std::function<void()>& callback, bool callEnterOnSceneLoad)
{
    mSceneToLoad = name;
    mSceneLoadedCallback = callback;
    mCallEnterOnSceneLoad = callEnterOnSceneLoad;
}

void SceneManager::UnloadScene(const std::function<void()>& callback)
{
    mUnloadScene = true;
    mSceneUnloadedCallback = callback;
}

void SceneManager::LoadSceneInternal()
{
    // Create the new scene.
    mScene = new Scene(mSceneToLoad);

    // Initiate scene load on background thread.
    mSceneLoading = true;
    Loader::Load([this](){
        TIMER_SCOPED("GEngine::LoadSceneInternal::Load");
        mScene->Load();
    });

    // Wait for background loading to complete.
    Loader::DoAfterLoading([this](){

        // Init the scene.
        // This is basically a continuation of the scene "Load", but with stuff to do on the main thread.
        mScene->Init();

        // If desired (usually yes), call the scene "Enter" function.
        // This acts like we just entered the scene from some other location. Usually the case, but not for loading save games.
        if(mCallEnterOnSceneLoad)
        {
            mScene->Enter();
        }

        // Done loading.
        mSceneLoading = false;

        // Execute scene load callback, if any.
        if(mSceneLoadedCallback != nullptr)
        {
            mSceneLoadedCallback();
            mSceneLoadedCallback = nullptr;
        }
    });
}

void SceneManager::UnloadSceneInternal()
{
    if(mScene != nullptr)
    {
        mScene->Unload();
        delete mScene;
        mScene = nullptr;
    }

    // Destroy any actors that are destroy on load.
    for(auto& actor : mActors)
    {
        if(actor->IsDestroyOnLoad())
        {
            actor->Destroy();
        }
    }

    // After destroy pass, delete destroyed actors.
    DeleteDestroyedActors();

    // Unload any assets scoped to just the current scene.
    gAssetManager.UnloadAssets(AssetScope::Scene);

    // Do callback if any.
    if(mSceneUnloadedCallback != nullptr)
    {
        std::function<void()> callback = mSceneUnloadedCallback;
        mSceneUnloadedCallback = nullptr;
        callback();
    }
}

void SceneManager::DeleteDestroyedActors()
{
    //TODO: Maybe switch to a "swap to end then delete" strategy.

    // Use iterator so we can carefully erase and delete actors without too many headaches.
    auto it = mActors.begin();
    while(it != mActors.end())
    {
        if((*it)->IsDestroyed())
        {
            Actor* actor = (*it);
            it = mActors.erase(it);
            delete actor;
        }
        else
        {
            ++it;
        }
    }
}
