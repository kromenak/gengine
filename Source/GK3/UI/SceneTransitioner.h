//
// Clark Kromenaker
//
// The "scene transitioner" is simply a fullscreen overlay that appears during scene loading.
//
#pragma once
#include "Actor.h"

class UIImage;

class SceneTransitioner : public Actor
{
public:
    SceneTransitioner();

    void Show();
    void Hide();

private:
    UIImage* mOverlay = nullptr;
};