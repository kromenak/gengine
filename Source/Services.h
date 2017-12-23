//
// Services.h
//
// Clark Kromenaker
//
// A locator for globally available services in the game.
//
#pragma once

class InputManager;
class SDLRenderer;

class Services
{
public:
    static InputManager* GetInput() { return inputManager; }
    static void SetInput(InputManager* input) { inputManager = input; }
    
    static SDLRenderer* GetRenderer() { return renderer; }
    static void SetRenderer(SDLRenderer* rend) { renderer = rend; }
    
private:
    static InputManager* inputManager;
    static SDLRenderer* renderer;
};
