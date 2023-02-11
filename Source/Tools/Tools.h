//
// Clark Kromenaker
//
// Entry point & high-level management of engine tools.
// Currently implemented with imgui.
//
// This system is similar to "construction mode" in the original game,
// but with a more modern UI and perhaps additional tools.
//
#pragma once

union SDL_Event;

namespace Tools
{
    void Init();
    void Shutdown();

    void Update();
    void Render();

    void SetActive(bool active);
    
    void ProcessEvent(const SDL_Event& event);
    bool EatingMouseInputs();
    bool EatingKeyboardInputs();
}