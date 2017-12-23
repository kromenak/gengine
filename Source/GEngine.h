//
//  GEngine.h
//  GEngine
//
//  Created by Clark Kromenaker on 7/22/17.
//
#pragma once

#include "SDLRenderer.h"
#include "SDLAudio.h"
#include "AssetManager.h"
#include "InputManager.h"
#include <vector>

class Actor;

class GEngine
{
public:
    GEngine();
    
    bool Initialize();
    void Shutdown();
    void Run();
    
    void Quit();
    
    static void AddActor(Actor* actor);
    static void RemoveActor(Actor* actor);
    
private:
    static std::vector<Actor*> mActors;
    
    bool mRunning;
    
    SDLRenderer mRenderer;
    SDLAudio mAudio;
    
    AssetManager mAssetManager;
    InputManager mInputManager;
    
    void ProcessInput();
    void Update();
    void GenerateOutput();
};
