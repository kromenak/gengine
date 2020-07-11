//
// StatusOverlay.h
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
};
