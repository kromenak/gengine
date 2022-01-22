//
// Clark Kromenaker
//
// UI that shows options during gameplay.
// Appears on right-click and shows buttons for inventory, options, quit, etc.
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
class UISlider;
class UIToggle;

class OptionBar : public Actor
{
public:
	OptionBar();
	
    void Show();
    void Hide();
    
protected:
	void OnUpdate(float deltaTime) override;
	
private:
    // Canvas for the option bar - takes up the entire screen.
    UICanvas* mCanvas = nullptr;
    
    // A large clickable area behind the action bar that stops scene interaction while visible.
    UIButton* mSceneBlockerButton = nullptr;

    // Main section.
    RectTransform* mOptionBarRoot = nullptr;
    UILabel* mScoreLabel = nullptr;
    UILabel* mDayLabel = nullptr;
    UILabel* mTimeLabel = nullptr;
    UIButton* mActiveInventoryItemButton = nullptr;

    UIButton* mInventoryButton = nullptr;
    UIButton* mHintButton = nullptr;
    UIButton* mCamerasButton = nullptr;
    UIButton* mHelpButton = nullptr;
    UIButton* mOptionsButton = nullptr;
    
    // Camera section.
    Actor* mCamerasSection = nullptr;

    // Options section.
    Actor* mOptionsSection = nullptr;
    UISlider* mGlobalVolumeSlider = nullptr;

    // Advanced Options section.
    Actor* mAdvancedOptionsSection = nullptr;

    // Sound Options section.
    Actor* mSoundOptionsSection = nullptr;
    UIToggle* mGlobalMuteToggle = nullptr;
    UIToggle* mSfxMuteToggle = nullptr;
    UIToggle* mVoMuteToggle = nullptr;
    UIToggle* mMusicMuteToggle = nullptr;

    UISlider* mGlobalVolumeSliderSecondary = nullptr;
    UISlider* mSfxVolumeSlider = nullptr;
    UISlider* mVoVolumeSlider = nullptr;
    UISlider* mMusicVolumeSlider = nullptr;

    // Graphics Options section.
    Actor* mGraphicOptionsSection = nullptr;

    // Advanced Graphics Options section.
    Actor* mAdvancedGraphicOptionsSection = nullptr;
    UIToggle* mMipmapsToggle = nullptr;
    UIToggle* mTrilinearFilteringToggle = nullptr;

    // Game Options section.
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
    
    void OnSoundOptionsButtonPressed();
    void OnGlobalVolumeSliderValueChanged(float value);
};
