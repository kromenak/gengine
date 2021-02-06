//
// OptionBar.h
//
// Clark Kromenaker
//
// Description
//
#pragma once
#include "Actor.h"

#include <string>
#include <unordered_map>

#include "IniParser.h"

class RectTransform;
class UIButton;
class UICanvas;
class UILabel;

class OptionBar : public Actor
{
public:
	OptionBar();
	
    void Show();
    void Hide();
    
protected:
	void OnUpdate(float deltaTime) override;
	
private:
    UICanvas* mCanvas = nullptr;
    
    // A large clickable area behind the action bar that stops scene interaction while visible.
    UIButton* mSceneBlockerButton = nullptr;
    
    RectTransform* mOptionBarRoot = nullptr;
    
    // Main section's text fields.
    UILabel* mScoreLabel = nullptr;
    UILabel* mDayLabel = nullptr;
    UILabel* mTimeLabel = nullptr;
    
    // Camera section root.
    Actor* mCamerasSection = nullptr;
    
    // Roots for all options sections.
    Actor* mOptionsSection = nullptr;
    Actor* mAdvancedOptionsSection = nullptr;
    Actor* mSoundOptionsSection = nullptr;
    Actor* mGraphicOptionsSection = nullptr;
    Actor* mAdvancedGraphicOptionsSection = nullptr;
    Actor* mGameOptionsSection = nullptr;
    
    void KeepOnScreen();
    
    void CreateMainSection(UICanvas* canvas, std::unordered_map<std::string, IniKeyValue>& config);
    void CreateCamerasSection(UICanvas* canvas, std::unordered_map<std::string, IniKeyValue>& config);
    void CreateOptionsSection(UICanvas* canvas, std::unordered_map<std::string, IniKeyValue>& config);
    void CreateAdvancedOptionsSection(UICanvas* canvas, std::unordered_map<std::string, IniKeyValue>& config);
    void CreateSoundOptionsSection(UICanvas* canvas, std::unordered_map<std::string, IniKeyValue>& config);
    void CreateGraphicOptionsSection(UICanvas* canvas, std::unordered_map<std::string, IniKeyValue>& config);
    void CreateAdvancedGraphicOptionsSection(UICanvas* canvas, std::unordered_map<std::string, IniKeyValue>& config);
    void CreateGameOptionsSection(UICanvas* canvas, std::unordered_map<std::string, IniKeyValue>& config);
};
