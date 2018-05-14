//
// Services.h
//
// Clark Kromenaker
//
// A locator for globally available services in the game.
//
#pragma once
#include "AssetManager.h"
#include "InputManager.h"
#include "SDLRenderer.h"
#include "AudioManager.h"

class Services
{
public:
    static AssetManager* GetAssets() { return assetManager; }
    static void SetAssets(AssetManager* assets) { assetManager = assets; }
    
    static InputManager* GetInput() { return inputManager; }
    static void SetInput(InputManager* input) { inputManager = input; }
    
    static SDLRenderer* GetRenderer() { return renderer; }
    static void SetRenderer(SDLRenderer* rend) { renderer = rend; }
    
    static AudioManager* GetAudio() { return audio; }
    static void SetAudio(AudioManager* aud) { audio = aud; }
    
private:
    static AssetManager* assetManager;
    static InputManager* inputManager;
    static SDLRenderer* renderer;
    static AudioManager* audio;
};
