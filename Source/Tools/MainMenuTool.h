//
// Clark Kromenaker
//
// The "main menu" seen when you activate the tools system.
// Shows the menu bar and enables access to other tools.
//
#pragma once

class MainMenuTool
{
public:
    // Flags for whether any windows/tools are active.
    // These are here so they can be easily toggled by the main menu.
    // And they're public so they can easily be passed around the tool system.
    bool hierarchyToolActive = false;

    void Render();
};