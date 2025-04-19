//
// Clark Kromenaker
//
// The GPS overlay is a UI that appears when using the in-game GPS device.
// It appears in the upper-left corner and shows a character's GPS coordinates in the current scene.
//
#pragma once
#include "Actor.h"

class UIButton;
class UIImage;
class UILabel;

class GPSOverlay : public Actor
{
public:
    GPSOverlay();

    void Show();
    void Hide();

protected:
    void OnUpdate(float deltaTime) override;

private:
    // Labels showing current longitude and latitude.
    UILabel* mLonLabel = nullptr;
    UILabel* mLatLabel = nullptr;

    // Target reticule images on the GPS.
    UIImage* mVerticalLine = nullptr;
    UIImage* mHorizontalLine = nullptr;
    UIImage* mTargetSquare = nullptr;

    // A button to turn off the power (closes the GPS).
    UIButton* mPowerButton = nullptr;

    // When showing or hiding the GPS, there's a short delay where some UI elements are not displayed.
    // This simulates the power turning on or off.
    const float kPowerDelay = 1.0f;
    float mPowerDelayTimer = 0.0f;
    bool mPoweringOn = false;

    void SetPoweredOnUIVisible(bool visible);
};