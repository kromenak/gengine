//
// Clark Kromenaker
//
// UI that displays status (location, day, time, score) at top of screen during gameplay.
//
#pragma once
#include "Actor.h"

class UILabel;

class StatusOverlay : public Actor
{
public:
    StatusOverlay();

    void Refresh();

protected:
    void OnUpdate(float deltaTime) override;

private:
    // Label containing status text.
    UILabel* mStatusLabel = nullptr;

    // Status text will show for kShowTime seconds, with
    // fade out beginning kStartFadeTime seconds before the show time ends.
    const float kShowTime = 3.0f;
    const float kStartFadeTime = 1.0f;

    // Timer for showing/fading text.
    // This is reset when the label is hovered/activated in code.
    // It counts down in update and causes the text to fade out.
    float mShowTimer = kShowTime;
};
