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
#include <vector>

class Actor;

class GEngine
{
public:
    GEngine();
    
    //TODO: Return more meaningful status code.
    bool Initialize();
    void Shutdown();
    void Run();
    
    void Quit();
    
private:
    void ProcessInput();
    void Update();
    void GenerateOutput();
    
    SDLRenderer mRenderer;
    SDLAudio mAudio;
    
    AssetManager mAssetManager;
    
    bool mRunning;
    
    std::vector<Actor*> mActors;
};
