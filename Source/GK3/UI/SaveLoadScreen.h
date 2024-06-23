#pragma once
#include "Actor.h"

#include "LayerManager.h"

class UIButton;

class SaveLoadScreen : public Actor
{
public:
    SaveLoadScreen();

    void ShowSave();
    void ShowLoad();
    void Hide();

private:
    Layer mSaveLayer;
    Layer mLoadLayer;

    UIButton* mSaveButton = nullptr;
    UIButton* mLoadButton = nullptr;
};