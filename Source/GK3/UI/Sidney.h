//
// Clark Kromenaker
//
// UI for the Sidney computer system in GK3.
//
#pragma once
#include "Actor.h"

#include "SidneyEmail.h"
#include "SidneySearch.h"

class UIButton;
class UIImage;
class UILabel;

class Sidney : public Actor
{
public:
    Sidney();

    void Show();
    void Hide();

protected:
    void OnUpdate(float deltaTime) override;

private:
    // New email label.
    UILabel* mNewEmailLabel = nullptr;

    // Controls email label blink behavior.
    const float kNewEmailBlinkInterval = 0.5f;
    float mNewEmailBlinkTimer = -1.0f;

    // Various subscreens.
    SidneySearch mSearch;
    SidneyEmail mEmail;
};