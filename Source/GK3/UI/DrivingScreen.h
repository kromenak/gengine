//
// Clark Kromenaker
//
// The driving screen shows destinations to travel to, as well as other people who are driving.
//
#pragma once
#include "Actor.h"

#include <string>
#include <unordered_map>

class UIButton;
class UICanvas;

class DrivingScreen : public Actor
{
public:
    DrivingScreen();

    void Show();
    void Hide();

private:
    // The canvas for this screen.
    UICanvas* mCanvas = nullptr;

    // The map is a child of the screen, and then all the location buttons are children of the map.
    Actor* mMapActor = nullptr;

    // Maps each location to its button.
    std::unordered_map<std::string, UIButton*> mLocationButtons;

    void AddLocation(const std::string& locationCode, const std::string& buttonId, const Vector2& buttonPos);
};