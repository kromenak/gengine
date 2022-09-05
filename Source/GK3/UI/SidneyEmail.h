//
// Clark Kromenaker
//
// UI for the email subscreen in Sidney.
//
#pragma once
#include "Actor.h"

class SidneyEmail
{
public:
    void Init(Actor* parent);

    void Show();
    void Hide();

    //void OnUpdate(float deltaTime);

private:
    // Root of this subscreen.
    Actor* mRoot = nullptr;

    // Root of the email list window.
    Actor* mEmailList = nullptr;
};