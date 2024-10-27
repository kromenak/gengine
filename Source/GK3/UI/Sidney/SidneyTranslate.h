//
// Clark Kromenaker
//
// UI for the "Translate" subscreen of Sidney.
//
#pragma once
#include "SidneyMenuBar.h"

class Actor;
class SidneyFiles;
class SidneyPopup;

class SidneyTranslate
{
public:
    void Init(Actor* parent, SidneyFiles* sidneyFiles);

    void Show();
    void Hide();

    void OnUpdate(float deltaTime);

private:
    // References to Sidney modules.
    SidneyFiles* mSidneyFiles = nullptr;

    // Root of this subscreen.
    Actor* mRoot = nullptr;

    // The menu bar.
    SidneyMenuBar mMenuBar;

    // ID of file currently being translated.
    int mTranslateFileId = -1;

    // A popup for displaying messages on this screen.
    SidneyPopup* mPopup = nullptr;
};