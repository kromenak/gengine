//
// Clark Kromenaker
//
// UI for the Sidney computer system in GK3.
//
#pragma once
#include "Actor.h"

#include "SidneySearch.h"

class UIButton;
class UICanvas;
class UIImage;

class Sidney : public Actor
{
public:
    Sidney();

    void Show();
    void Hide();

private:
    // The canvas for this screen.
    UICanvas* mCanvas = nullptr;

    // Various subscreens.
    SidneySearch mSearch;
};