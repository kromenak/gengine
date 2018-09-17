//
// Services.h
//
// Clark Kromenaker
//
// A locator for globally available services in the game.
//
#pragma once
#include "AssetManager.h"
#include "AudioManager.h"
#include "InputManager.h"
#include "Renderer.h"
#include "SheepManager.h"

class Services
{
public:
    static AssetManager* GetAssets() { return sAssetManager; }
    static void SetAssets(AssetManager* assets) { sAssetManager = assets; }
    
    static InputManager* GetInput() { return sInputManager; }
    static void SetInput(InputManager* input) { sInputManager = input; }
    
    static Renderer* GetRenderer() { return sRenderer; }
    static void SetRenderer(Renderer* rend) { sRenderer = rend; }
    
    static AudioManager* GetAudio() { return sAudio; }
    static void SetAudio(AudioManager* aud) { sAudio = aud; }
    
    static SheepManager* GetSheep() { return sSheep; }
    static void SetSheep(SheepManager* shp) { sSheep = shp; }
    
private:
    static AssetManager* sAssetManager;
    static InputManager* sInputManager;
    static Renderer* sRenderer;
    static AudioManager* sAudio;
    static SheepManager* sSheep;
};
