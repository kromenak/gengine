#include "OptionBar.h"

#include "ActionManager.h"
#include "AssetManager.h"
#include "AudioManager.h"
#include "CaptionsOverlay.h"
#include "GameCamera.h"
#include "GameProgress.h"
#include "GEngine.h"
#include "GK3UI.h"
#include "HintManager.h"
#include "IniParser.h"
#include "InputManager.h"
#include "InventoryManager.h"
#include "LayerManager.h"
#include "LocationManager.h"
#include "Renderer.h"
#include "TextAsset.h"
#include "Tools.h"
#include "UIButton.h"
#include "UIDrag.h"
#include "UIDropdown.h"
#include "UIImage.h"
#include "UILabel.h"
#include "UISlider.h"
#include "UIToggle.h"
#include "UIUtil.h"
#include "Window.h"

namespace
{
    UIButton* CreateButton(std::unordered_map<std::string, IniKeyValue>& config, const std::string& buttonId, Actor* parent, bool setSprites = true)
    {
        UIButton* button = UI::CreateWidgetActor<UIButton>(buttonId, parent);
        button->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);

        Vector2 buttonPos = config[buttonId + "Pos"].GetValueAsVector2();
        buttonPos.y *= -1;
        button->GetRectTransform()->SetAnchoredPosition(buttonPos);

        button->SetTooltipText("tb_" + buttonId);

        if(setSprites)
        {
            auto it = config.find(buttonId + "SpriteUp");
            if(it != config.end())
            {
                button->SetUpTexture(gAssetManager.LoadTexture(it->second.value));
            }

            it = config.find(buttonId + "SpriteDown");
            if(it != config.end())
            {
                button->SetDownTexture(gAssetManager.LoadTexture(it->second.value));
            }

            it = config.find(buttonId + "SpriteDis");
            if(it != config.end())
            {
                button->SetDisabledTexture(gAssetManager.LoadTexture(it->second.value));
            }

            it = config.find(buttonId + "SpriteHov");
            if(it != config.end())
            {
                button->SetHoverTexture(gAssetManager.LoadTexture(it->second.value));
            }
        }
        return button;
    }

    UIToggle* CreateToggle(std::unordered_map<std::string, IniKeyValue>& config, const std::string& toggleId, Actor* parent)
    {
        UIToggle* toggle = UI::CreateWidgetActor<UIToggle>(toggleId, parent);
        toggle->GetRectTransform()->SetAnchor(0.0f, 1.0f);
        toggle->GetRectTransform()->SetPivot(0.0f, 1.0f);

        Vector2 togglePos = config[toggleId + "Pos"].GetValueAsVector2();
        togglePos.y *= -1;
        toggle->GetRectTransform()->SetAnchoredPosition(togglePos);

        toggle->SetTooltipText("tb_" + toggleId);

        toggle->SetOnTexture(gAssetManager.LoadTexture(config[toggleId + "SpriteDown"].value));
        toggle->SetOffTexture(gAssetManager.LoadTexture(config[toggleId + "SpriteUp"].value));
        toggle->SetDisabledTexture(gAssetManager.LoadTexture(config[toggleId + "SpriteDis"].value));
        return toggle;
    }

    UISlider* CreateSlider(std::unordered_map<std::string, IniKeyValue>& config, const std::string& sliderId, Actor* parent)
    {
        UISlider* slider = UI::CreateWidgetActor<UISlider>(sliderId, parent);
        slider->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);

        // Set slider min/max positions.
        // This makes the slider's rect transform the exact width required for the min/max slider positions.
        Vector2 minPos = config[sliderId + "Min"].GetValueAsVector2();
        minPos.y *= -1;
        slider->GetRectTransform()->SetAnchoredPosition(minPos);

        Vector2 maxPos = config[sliderId + "Max"].GetValueAsVector2();
        maxPos.y *= -1;

        // Add handle image.
        UIImage* handleImage = UI::CreateWidgetActor<UIImage>("Handle", slider);
        handleImage->SetTexture(gAssetManager.LoadTexture("RC_SO_SLIDER"), true);
        slider->SetHandleActor(handleImage->GetOwner());

        // Set slider's vertical size to be exactly equal to handle size.
        // This should make it so the handle can only move left/right.
        Vector2 handleSize = handleImage->GetRectTransform()->GetSize();
        slider->GetRectTransform()->SetSizeDeltaY(handleSize.y);

        // Slider's horizontal size is slider width, PLUS width of the handle itself.
        float sliderWidth = (maxPos.x - minPos.x);
        slider->GetRectTransform()->SetSizeDeltaX(sliderWidth + handleSize.x);
        return slider;
    }
}

OptionBar::OptionBar() : Actor("OptionBar", TransformType::RectTransform)
{
    // Load layout text file, parse to key/value map, and then delete it.
    TextAsset* optionBarText = gAssetManager.LoadText("RC_LAYOUT.TXT", AssetScope::Manual);

    IniParser parser(optionBarText->GetText(), optionBarText->GetTextLength());
    parser.SetMultipleKeyValuePairsPerLine(false);
    std::unordered_map<std::string, IniKeyValue> optionBarConfig = parser.ParseAllAsMap();

    // Create canvas, to contain the UI components.
    UI::AddCanvas(this, 11);

    // The background of the action bar consists of a fullscreen clickable button area.
    // This stops interaction with the scene while action bar is visible.
    mSceneBlockerButton = AddComponent<UIButton>();
    mSceneBlockerButton->SetPressCallback([this](UIButton* button) {
        Hide();
    });

    // Create "more options" button. These are options that weren't included in the original game.
    mMoreOptionsButton = UI::CreateWidgetActor<UIButton>("MoreOptions", this);
    mMoreOptionsButton->GetRectTransform()->SetAnchor(AnchorPreset::BottomRight);
    mMoreOptionsButton->SetUpTexture(gAssetManager.LoadTexture("I_OPERATE_STD.BMP"));
    mMoreOptionsButton->SetHoverTexture(gAssetManager.LoadTexture("I_OPERATE_HOV.BMP"));
    mMoreOptionsButton->SetDownTexture(gAssetManager.LoadTexture("I_OPERATE_DWN.BMP"));
    mMoreOptionsButton->SetTooltipText("tb_optAdvanced");
    mMoreOptionsButton->SetPressCallback([](UIButton* button){
        Tools::ShowSettings();
    });

    // Create sections.
    CreateMainSection(optionBarConfig);
    CreateCamerasSection(optionBarConfig);
    CreateOptionsSection(optionBarConfig);

    // Hide by default.
    Hide();

    delete optionBarText;
}

void OptionBar::Show()
{
    SetActive(true);

    // Only show the "active inventory item" button if there is an active inventory item.
    std::string activeInvItem = gInventoryManager.GetActiveInventoryItem();
    mActiveInventoryItemButton->SetEnabled(!activeInvItem.empty());
    if(!activeInvItem.empty())
    {
        mActiveInventoryItemButton->SetUpTexture(gInventoryManager.GetInventoryItemIconTexture(activeInvItem));
    }

    // Update score label.
    mScoreLabel->SetText(StringUtil::Format("%03i/%03i",
                                            gGameProgress.GetScore(),
                                            gGameProgress.GetMaxScore()));

    // The state of the cinematics button depends on whether cinematics are enabled or disabled.
    RefreshCinematicsButtonState();

    // The radio button only appears in Day 3, 9PM timeblock and hides the "hint button.
    mRadioButton->SetEnabled(gGameProgress.GetTimeblock() == Timeblock(3, 21));

    // The exit inventory button only shows when the inventory is open.
    mExitInventoryButton->SetEnabled(gInventoryManager.IsInventoryShowing());

    // The "exit this screen" button only shows when select UIs are visible.
    mExitScreenButton->SetEnabled(gGK3UI.IsOnExitableScreen());

    // Position option bar over mouse.
    {
        // Option bar pivot is in lower-left corner, but want mouse at center.
        // So get "half size" to offset by that amount from mouse cursor.
        Vector2 halfSize = mOptionBarRoot->GetSize() * 0.5f;

        // Also, round the "half size" to ensure the UI renders "pixel perfect" - on exact pixel spots rather than between them.
        halfSize.x = Math::Round(halfSize.x);
        halfSize.y = Math::Round(halfSize.y);

        // Due to possible parent transform scaling, we can't assume the world space matches this canvas's space.
        // So, convert mouse world position to local space.
        Vector3 localMousePos = GetTransform()->GetWorldToLocalMatrix().TransformPoint(gInputManager.GetMousePosition());

        // Set option bar to mouse pos minus half its size.
        mOptionBarRoot->SetAnchoredPosition(localMousePos - halfSize);
    }

    // Force to be fully on screen.
    KeepOnScreen();

    // When the option bar is showing, you are allowed to interact with it, even during an action.
    GEngine::Instance()->SetAllowInteractDuringActions(true);
}

void OptionBar::Hide()
{
    // Reset sections to hidden.
    mCamerasSection->SetActive(false);
    mOptionsSection->SetActive(false);
    mAdvancedOptionsSection->SetActive(false);
    mSoundOptionsSection->SetActive(false);
    mGraphicOptionsSection->SetActive(false);
    mAdvancedGraphicOptionsSection->SetActive(false);
    mGameOptionsSection->SetActive(false);

    // Hide the main section.
    SetActive(false);

    // Since we're closing this bar, stop allowing interacting during actions.
    GEngine::Instance()->SetAllowInteractDuringActions(false);
}

bool OptionBar::CanShow() const
{
    // Showing the option bar is often allowed, but there are a few screens where it isn't allowed.
    if(gLayerManager.IsTopLayer("PauseLayer") ||
       gLayerManager.IsTopLayer("ConfirmQuitLayer") ||
       gLayerManager.IsTopLayer("HelpLayer") ||
       gLayerManager.IsLayerInStack("SaveLayer") || gLayerManager.IsLayerInStack("LoadLayer") ||
       gLayerManager.IsTopLayer("DeathLayer") ||
       gLayerManager.IsTopLayer("BinocsLayer"))
    {
        return false;
    }

    // Allowed to open options bar.
    return true;
}

void OptionBar::OnUpdate(float deltaTime)
{
    bool actionActive = gActionManager.IsActionPlaying();
    bool onDrivingScreen = gGK3UI.IsOnDrivingScreen();

    // Inventory buttons can't be interacted with during actions and on driving screen.
    mActiveInventoryItemButton->SetCanInteract(!actionActive && !onDrivingScreen);
    mInventoryButton->SetCanInteract(!actionActive && !onDrivingScreen);

    // Hint button is only available when a hint is available (and no action is happening).
    mHintButton->SetCanInteract(!actionActive && gHintManager.IsHintAvailable());

    // Radio button is only interactive when no action is occurring.
    mRadioButton->SetCanInteract(!actionActive);

    // Camera/help buttons are only available when not doing actions and not on driving screen.
    mCamerasButton->SetCanInteract(!actionActive && !onDrivingScreen);
    mHelpButton->SetCanInteract(!actionActive && !onDrivingScreen);

    // Options can't be accessed during actions or on driving screen.
    mOptionsButton->SetCanInteract(!actionActive && !onDrivingScreen);

    // Cinematic buttons are almost always interactive - the only time I've seen it not so was on the driving screen.
    mCinematicsOffButton->SetCanInteract(!gGK3UI.IsOnDrivingScreen());
    mCinematicsOnButton->SetCanInteract(!gGK3UI.IsOnDrivingScreen());

    // Note: Close button is always interactive.

    // Only show the "more options" button if the advanced options section is open.
    mMoreOptionsButton->SetEnabled(mAdvancedOptionsSection->IsActive());

    // Most keyboard input counts as a cancel action, unless some text input is active (like debug window).
    // Any key press EXCEPT ~ counts as a cancel action.
    // This logic technically blocks any other cancel action due to key presses WHILE ~ is pressed but...close enough.
    if(gGK3UI.IsAnyKeyPressedOutsideTextInputAndConsole())
    {
        Hide();
    }

    // The option bar should always stay on-screen, so run the "keep on screen" logic every frame.
    // This also forces the bar to stay on screen if the resolution is changed by the player.
    KeepOnScreen();
}

void OptionBar::KeepOnScreen()
{
    // Make sure the options bar stays on-screen.
    mOptionBarRoot->MoveInsideRect(Window::GetRect());
}

void OptionBar::CreateMainSection(std::unordered_map<std::string, IniKeyValue>& config)
{
    // Add background image.
    UIImage* backgroundImage = UI::CreateWidgetActor<UIImage>("Window", this);
    backgroundImage->SetTexture(gAssetManager.LoadTexture(config["backSprite"].value), true);
    mOptionBarRoot = backgroundImage->GetRectTransform();
    mOptionBarRoot->SetAnchor(AnchorPreset::BottomLeft);

    // The background is draggable.
    UIDrag* drag = backgroundImage->GetOwner()->AddComponent<UIDrag>();
    drag->SetBoundaryRectTransform(GetComponent<RectTransform>());
    drag->SetUseHighlightCursor(false); // but don't highlight cursor when hovering it

    // Load font.
    Font* font = gAssetManager.LoadFont(config["statusFont"].value);

    // Add score text.
    mScoreLabel = UI::CreateWidgetActor<UILabel>("ScoreLabel", mOptionBarRoot);
    mScoreLabel->GetRectTransform()->SetSizeDelta(config["scoreSize"].GetValueAsVector2());
    mScoreLabel->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);

    Vector2 scorePos = config["scorePos"].GetValueAsVector2();
    scorePos.y *= -1;
    mScoreLabel->GetRectTransform()->SetAnchoredPosition(scorePos);

    mScoreLabel->SetFont(font);
    mScoreLabel->SetHorizonalAlignment(HorizontalAlignment::Center);
    mScoreLabel->SetVerticalAlignment(VerticalAlignment::Top);
    mScoreLabel->SetMasked(true);

    // Determine display strings for the day/time.
    // We only have to do this on creation (and not show) because the day/time never change in the middle of a scene.
    std::vector<std::string> dayAndTimeStrings = StringUtil::Split(gGameProgress.GetTimeblockDisplayName(), ',');
    StringUtil::Trim(dayAndTimeStrings[0]);
    StringUtil::Trim(dayAndTimeStrings[1]);

    // Add day text.
    UILabel* dayLabel = UI::CreateWidgetActor<UILabel>("Day", mOptionBarRoot);
    dayLabel->GetRectTransform()->SetSizeDelta(config["daySize"].GetValueAsVector2());
    dayLabel->GetRectTransform()->SetAnchor(0.0f, 1.0f);
    dayLabel->GetRectTransform()->SetPivot(0.0f, 1.0f);

    Vector2 dayPos = config["dayPos"].GetValueAsVector2();
    dayPos.y *= -1;
    dayLabel->GetRectTransform()->SetAnchoredPosition(dayPos);

    dayLabel->SetFont(font);
    dayLabel->SetText(dayAndTimeStrings[0]);
    dayLabel->SetHorizonalAlignment(HorizontalAlignment::Center);
    dayLabel->SetVerticalAlignment(VerticalAlignment::Top);
    dayLabel->SetMasked(true);

    // Add time text.
    UILabel* timeLabel = UI::CreateWidgetActor<UILabel>("Time", mOptionBarRoot);
    timeLabel->GetRectTransform()->SetSizeDelta(config["timeSize"].GetValueAsVector2());
    timeLabel->GetRectTransform()->SetAnchor(0.0f, 1.0f);
    timeLabel->GetRectTransform()->SetPivot(0.0f, 1.0f);

    Vector2 timePos = config["timePos"].GetValueAsVector2();
    timePos.y *= -1;
    timeLabel->GetRectTransform()->SetAnchoredPosition(timePos);

    timeLabel->SetFont(font);
    timeLabel->SetText(dayAndTimeStrings[1]);
    timeLabel->SetHorizonalAlignment(HorizontalAlignment::Center);
    timeLabel->SetVerticalAlignment(VerticalAlignment::Top);
    timeLabel->SetMasked(true);

    // Add active inventory item button.
    mActiveInventoryItemButton = CreateButton(config, "currInv", mOptionBarRoot->GetOwner());
    mActiveInventoryItemButton->SetPressCallback([this](UIButton* button) {
        Hide();

        // This button usually shows the inventory inspect/closeup of the active inventory item.
        // But it has an odd behavior if the closeup is already active: it closes it!
        if(!gInventoryManager.IsInventoryInspectShowing())
        {
            gInventoryManager.InventoryInspect();
        }
        else
        {
            gInventoryManager.InventoryUninspect();
        }
    });

    // Since the active inventory item may not be set when first starting the game, zero the size until an image is set.
    // This fixes a bug where the default size of 100x100 causes the option bar to be taller than it visually seems to be, which affects dragging boundaries.
    mActiveInventoryItemButton->GetRectTransform()->SetSizeDelta(Vector2::Zero);

    // Add inventory button.
    mInventoryButton = CreateButton(config, "closed", mOptionBarRoot->GetOwner());
    mInventoryButton->SetPressCallback([this](UIButton* button) {
        Hide();
        gInventoryManager.ShowInventory();
    });

    // Add close inventory button.
    // This button only appears when the inventory is visible, and allows you to exit the inventory via the option bar.
    mExitInventoryButton = CreateButton(config, "open", mOptionBarRoot->GetOwner());
    mExitInventoryButton->SetPressCallback([this](UIButton* button){
        Hide();
        gInventoryManager.HideInventory();
    });

    // Add exit screen button.
    // This button only appears when exitable screens are up, and allows you to exit that screen.
    mExitScreenButton = CreateButton(config, "invExit", mOptionBarRoot->GetOwner());
    mExitScreenButton->SetPressCallback([this](UIButton* button){
        Hide();
        gGK3UI.ExitCurrentScreen();
    });

    // Add hint button.
    mHintButton = CreateButton(config, "hint", mOptionBarRoot->GetOwner());
    mHintButton->SetPressCallback([this](UIButton* button) {
        Hide();
        gHintManager.ShowHint();
    });

    // Add radio button.
    mRadioButton = CreateButton(config, "radio", mOptionBarRoot->GetOwner());
    mRadioButton->SetPressCallback([this](UIButton* button){
        OnRadioButtonPressed();
    });

    // Add camera button.
    mCamerasButton = CreateButton(config, "camera", mOptionBarRoot->GetOwner());
    mCamerasButton->SetPressCallback([this](UIButton* button) {
        mCamerasSection->SetActive(!mCamerasSection->IsActive());
        mOptionsSection->SetActive(false);
    });

    // Add cinematics button.
    mCinematicsOffButton = CreateButton(config, "cine", mOptionBarRoot->GetOwner());
    mCinematicsOffButton->SetPressCallback(std::bind(&OptionBar::OnCinematicsButtonPressed, this, std::placeholders::_1));

    mCinematicsOnButton = CreateButton(config, "cineoff", mOptionBarRoot->GetOwner());
    mCinematicsOnButton->SetPressCallback(std::bind(&OptionBar::OnCinematicsButtonPressed, this, std::placeholders::_1));
    RefreshCinematicsButtonState();

    // Add help button.
    mHelpButton = CreateButton(config, "help", mOptionBarRoot->GetOwner());
    mHelpButton->SetPressCallback([this](UIButton* button) {
        Hide();
        gGK3UI.ShowHelpScreen();
    });

    // Add options button.
    mOptionsButton = CreateButton(config, "options", mOptionBarRoot->GetOwner());
    mOptionsButton->SetPressCallback([this](UIButton* button) {
        mCamerasSection->SetActive(false);
        mOptionsSection->SetActive(!mOptionsSection->IsActive());
        mGlobalVolumeSlider->SetValueSilently(gAudioManager.GetMasterVolume());
    });

    // Add close button.
    UIButton* closeButton = CreateButton(config, "exit", mOptionBarRoot->GetOwner());
    closeButton->SetPressCallback([this](UIButton* button) {
        Hide();
    });
}

void OptionBar::CreateCamerasSection(std::unordered_map<std::string, IniKeyValue>& config)
{
    // Add background image.
    UIImage* backgroundImage = UI::CreateWidgetActor<UIImage>("CamerasSection", mOptionBarRoot);
    backgroundImage->SetTexture(gAssetManager.LoadTexture(config["camBackSprite"].value), true);
    mCamerasSection = backgroundImage->GetOwner();

    // Position directly below main section.
    backgroundImage->GetRectTransform()->SetAnchor(0.0f, 0.0f);
    backgroundImage->GetRectTransform()->SetPivot(0.0f, 1.0f);
    backgroundImage->SetReceivesInput(true);
}

void OptionBar::CreateOptionsSection(std::unordered_map<std::string, IniKeyValue>& config)
{
    // Add background image.
    UIImage* backgroundImage = UI::CreateWidgetActor<UIImage>("OptionsSection", mOptionBarRoot);
    backgroundImage->SetTexture(gAssetManager.LoadTexture(config["optBackSprite"].value), true);
    mOptionsSection = backgroundImage->GetOwner();

    // Position directly below main section.
    backgroundImage->GetRectTransform()->SetAnchor(0.0f, 0.0f);
    backgroundImage->GetRectTransform()->SetPivot(0.0f, 1.0f);
    backgroundImage->SetReceivesInput(true);

    // Create volume slider.
    mGlobalVolumeSlider = CreateSlider(config, "optSlider", mOptionsSection);
    mGlobalVolumeSlider->SetValueChangeCallback(std::bind(&OptionBar::OnGlobalVolumeSliderValueChanged, this, std::placeholders::_1));

    // Create save button.
    UIButton* saveButton = CreateButton(config, "optSave", mOptionsSection);
    saveButton->SetPressCallback([this](UIButton* button) {
        Hide();
        gGK3UI.ShowSaveScreen();
    });

    // Create restore button.
    UIButton* restoreButton = CreateButton(config, "optRestore", mOptionsSection);
    restoreButton->SetPressCallback([this](UIButton* button) {
        Hide();
        gGK3UI.ShowLoadScreen();
    });

    // Created "advanced options" button.
    UIButton* advOptButton = CreateButton(config, "optAdvanced", mOptionsSection);
    advOptButton->SetPressCallback([this](UIButton* button) {
        mAdvancedOptionsSection->SetActive(!this->mAdvancedOptionsSection->IsActive());
        KeepOnScreen();
    });

    // Create "quit game" button.
    UIButton* quitButton = CreateButton(config, "optQuit", mOptionsSection);
    quitButton->SetPressCallback([this](UIButton* button) {
        Hide();
        gGK3UI.ShowQuitPopup();
    });

    // Create advanced options section.
    CreateAdvancedOptionsSection(config);
}

void OptionBar::CreateAdvancedOptionsSection(std::unordered_map<std::string, IniKeyValue>& config)
{
    // Add background image.
    UIImage* backgroundImage = UI::CreateWidgetActor<UIImage>("AdvOptionsSection", mOptionsSection);
    backgroundImage->SetTexture(gAssetManager.LoadTexture(config["advOptBackSprite"].value), true);
    mAdvancedOptionsSection = backgroundImage->GetOwner();

    // Position directly below main section.
    backgroundImage->GetRectTransform()->SetAnchor(0.0f, 0.0f);
    backgroundImage->GetRectTransform()->SetPivot(0.0f, 1.0f);
    backgroundImage->SetReceivesInput(true);

    // Create sound options button.
    UIButton* soundOptsButton = CreateButton(config, "advOptSound", mAdvancedOptionsSection);
    soundOptsButton->SetPressCallback(std::bind(&OptionBar::OnSoundOptionsButtonPressed, this, std::placeholders::_1));

    // Create graphics options button.
    UIButton* graphicsOptsButton = CreateButton(config, "advOptGraphics", mAdvancedOptionsSection);
    graphicsOptsButton->SetPressCallback(std::bind(&OptionBar::OnGraphicsOptionsButtonPressed, this, std::placeholders::_1));

    // Create game options button.
    UIButton* gameOptsButton = CreateButton(config, "advOptGame", mAdvancedOptionsSection);
    gameOptsButton->SetPressCallback(std::bind(&OptionBar::OnGameOptionsButtonPressed, this, std::placeholders::_1));

    // Create each subsection...
    CreateSoundOptionsSection(config);
    CreateGraphicOptionsSection(config);
    CreateGameOptionsSection(config);
}

void OptionBar::CreateSoundOptionsSection(std::unordered_map<std::string, IniKeyValue>& config)
{
    // Add background image.
    UIImage* backgroundImage = UI::CreateWidgetActor<UIImage>("SoundOptions", mAdvancedOptionsSection);
    backgroundImage->SetTexture(gAssetManager.LoadTexture(config["soundOptBackSprite"].value), true);
    mSoundOptionsSection = backgroundImage->GetOwner();

    // Position directly below main section.
    backgroundImage->GetRectTransform()->SetAnchor(0.0f, 0.0f);
    backgroundImage->GetRectTransform()->SetPivot(0.0f, 1.0f);
    backgroundImage->SetReceivesInput(true);

    // Create global volume slider.
    mGlobalVolumeSliderSecondary = CreateSlider(config, "optSoundGlobalSlider", mSoundOptionsSection);
    mGlobalVolumeSliderSecondary->SetValueChangeCallback(std::bind(&OptionBar::OnGlobalVolumeSliderValueChanged, this, std::placeholders::_1));

    // Create global mute toggle.
    mGlobalMuteToggle = CreateToggle(config, "soundOptGlobal", mSoundOptionsSection);
    mGlobalMuteToggle->SetToggleCallback([](bool isOn) {
        gAudioManager.SetMuted(isOn);
    });

    // Create sfx volume slider.
    mSfxVolumeSlider = CreateSlider(config, "optSoundSfxSlider", mSoundOptionsSection);
    mSfxVolumeSlider->SetValueChangeCallback([](float value) {
        // The SFX volume slider also controls ambient volume.
        gAudioManager.SetVolume(AudioType::SFX, value);
        gAudioManager.SetVolume(AudioType::Ambient, value);
    });

    //Create sfx mute toggle.
    mSfxMuteToggle = CreateToggle(config, "soundOptSfx", mSoundOptionsSection);
    mSfxMuteToggle->SetToggleCallback([](bool isOn) {
        // The SFX toggle also controls ambient.
        gAudioManager.SetMuted(AudioType::SFX, isOn);
        gAudioManager.SetMuted(AudioType::Ambient, isOn);
    });

    // Create VO volume slider.
    mVoVolumeSlider = CreateSlider(config, "optSoundDialogueSlider", mSoundOptionsSection);
    mVoVolumeSlider->SetValueChangeCallback([](float value) {
        gAudioManager.SetVolume(AudioType::VO, value);
    });

    // Create VO mute toggle.
    mVoMuteToggle = CreateToggle(config, "soundOptDialogue", mSoundOptionsSection);
    mVoMuteToggle->SetToggleCallback([](bool isOn) {
        gAudioManager.SetMuted(AudioType::VO, isOn);
    });

    // Create music volume slider.
    mMusicVolumeSlider = CreateSlider(config, "optSoundMusicSlider", mSoundOptionsSection);
    mMusicVolumeSlider->SetValueChangeCallback([](float value) {
        gAudioManager.SetVolume(AudioType::Music, value);
    });

    // Create music mute toggle.
    mMusicMuteToggle = CreateToggle(config, "soundOptMusic", mSoundOptionsSection);
    mMusicMuteToggle->SetToggleCallback([](bool isOn) {
        gAudioManager.SetMuted(AudioType::Music, isOn);
    });

    // Create max sounds slider.
    UISlider* maxSoundsSlider = CreateSlider(config, "optSoundNumSlider", mSoundOptionsSection);
    maxSoundsSlider->SetValueChangeCallback([](float value) {
        std::cout << "Changed max sounds to " << value << std::endl;
    });
}

void OptionBar::CreateGraphicOptionsSection(std::unordered_map<std::string, IniKeyValue>& config)
{
    // Add background image.
    UIImage* backgroundImage = UI::CreateWidgetActor<UIImage>("GraphicsOptions", mAdvancedOptionsSection);
    backgroundImage->SetTexture(gAssetManager.LoadTexture(config["graphicsOptBackSprite"].value), true);
    mGraphicOptionsSection = backgroundImage->GetOwner();

    // Position directly below main section.
    backgroundImage->GetRectTransform()->SetAnchor(0.0f, 0.0f);
    backgroundImage->GetRectTransform()->SetPivot(0.0f, 1.0f);
    backgroundImage->SetReceivesInput(true);

    //TODO: Gamma slider

    //TODO: 3D driver dropdown

    // Create "incremental rendering" text (text can be "disabled", like a button, if graphics system doesn't support this option).
    UIButton* incRenderingText = CreateButton(config, "graphOptIncrementalText", mGraphicOptionsSection, false);
    incRenderingText->SetUpTexture(gAssetManager.LoadTexture(config["graphOptIncrementalEnabled"].value));
    incRenderingText->SetDisabledTexture(gAssetManager.LoadTexture(config["graphOptIncrementalDisabled"].value));
    incRenderingText->SetReceivesInput(false);

    // Create "incremental rendering" toggle.
    UIToggle* incRenderingToggle = CreateToggle(config, "graphOptIncremental", mGraphicOptionsSection);
    incRenderingToggle->SetToggleCallback([](bool isOn) {
        std::cout << "Incremental rendering is toggled " << isOn << std::endl;
    });

    // Create advanced graphics options button.
    UIButton* advGraphicsButton = CreateButton(config, "graphOptAdvanced", mGraphicOptionsSection);
    advGraphicsButton->SetPressCallback([this](UIButton* button) {
        mAdvancedGraphicOptionsSection->SetActive(!mAdvancedGraphicOptionsSection->IsActive());

        if(mAdvancedGraphicOptionsSection->IsActive())
        {
            mMipmapsToggle->SetValue(gRenderer.UseMipmaps());
            mTrilinearFilteringToggle->SetValue(gRenderer.UseTrilinearFiltering());
        }
    });

    CreateAdvancedGraphicOptionsSection(config);

    // Resolution dropdown (create after other elements so dropdown box draws above everything).
    mResolutionDropdown = new UIDropdown("ResolutionDropdown", this);
    mResolutionDropdown->SetMaxVisibleChoices(5);
    RectTransform* resolutionDropdownRT = mResolutionDropdown->GetComponent<RectTransform>();
    resolutionDropdownRT->SetParent(mGraphicOptionsSection->GetTransform());
    resolutionDropdownRT->SetAnchor(AnchorPreset::TopLeft);

    // Dropdown sizes in the config file seem wrong, at least based on what I'm expecting.
    // e.g. this one says height is 39, but the area in the art is clearly 15...
    Vector2 dropdownSize = config["graphOptResolutionBoxSize"].GetValueAsVector2();
    dropdownSize.y = 15;

    // Dropdown position is also a bit weird - the pos is from top-left, but it's the bottom-left corner pos.
    Vector2 dropdownPos = config["graphOptResolutionBoxPos"].GetValueAsVector2();
    dropdownPos.y -= dropdownSize.y;
    dropdownPos.y *= -1;

    resolutionDropdownRT->SetAnchoredPosition(dropdownPos);
    resolutionDropdownRT->SetSizeDelta(dropdownSize);

    mResolutionDropdown->SetCallback([](int selectedIndex){
        const std::vector<Window::Resolution>& resolutions = Window::GetResolutions();
        if(selectedIndex >= 0 && selectedIndex < resolutions.size())
        {
            gRenderer.ChangeResolution(resolutions[selectedIndex]);
        }
    });
}

void OptionBar::CreateAdvancedGraphicOptionsSection(std::unordered_map<std::string, IniKeyValue>& config)
{
    // Add background image.
    UIImage* backgroundImage = UI::CreateWidgetActor<UIImage>("AdvancedGraphicSection", mGraphicOptionsSection);
    backgroundImage->SetTexture(gAssetManager.LoadTexture(config["advGraphOptBackSprite"].value), true);
    mAdvancedGraphicOptionsSection = backgroundImage->GetOwner();

    // Position directly below main section.
    backgroundImage->GetRectTransform()->SetAnchor(0.0f, 0.0f);
    backgroundImage->GetRectTransform()->SetPivot(0.0f, 1.0f);
    backgroundImage->SetReceivesInput(true);

    //TODO: Texture quality dropdown
    //TODO: Lighting quality dropdown

    // Create "mipmap" text (text can be "disabled", like a button, if graphics system doesn't support this option).
    UIButton* mipmapText = CreateButton(config, "advGraphOptMipMapText", mAdvancedGraphicOptionsSection, false);
    mipmapText->SetUpTexture(gAssetManager.LoadTexture(config["advGraphOptMipMapEnabled"].value));
    mipmapText->SetDisabledTexture(gAssetManager.LoadTexture(config["advGraphOptMipMapDisabled"].value));
    mipmapText->SetReceivesInput(false);

    // Create "mipmap" toggle.
    mMipmapsToggle = CreateToggle(config, "advGraphOptMipMap", mAdvancedGraphicOptionsSection);
    mMipmapsToggle->SetToggleCallback([](bool isOn) {
        gRenderer.SetUseMipmaps(isOn);
    });

    // Create "animation interpolation" text (text can be "disabled", like a button, if graphics system doesn't support this option).
    UIButton* animInterpText = CreateButton(config, "advGraphOptInterpolationText", mAdvancedGraphicOptionsSection, false);
    animInterpText->SetUpTexture(gAssetManager.LoadTexture(config["advGraphOptInterpolationEnabled"].value));
    animInterpText->SetDisabledTexture(gAssetManager.LoadTexture(config["advGraphOptInterpolationDisabled"].value));
    animInterpText->SetReceivesInput(false);

    // Create "animation interpolation" toggle.
    UIToggle* animInterpToggle = CreateToggle(config, "advGraphOptInterpolation", mAdvancedGraphicOptionsSection);
    animInterpToggle->SetToggleCallback([](bool isOn) {
        std::cout << "Anim interp is toggled " << isOn << std::endl;
    });

    // Create "trilinear filtering" text (text can be "disabled", like a button, if graphics system doesn't support this option).
    UIButton* filteringText = CreateButton(config, "advGraphOptFilteringText", mAdvancedGraphicOptionsSection, false);
    filteringText->SetUpTexture(gAssetManager.LoadTexture(config["advGraphOptFilteringEnabled"].value));
    filteringText->SetDisabledTexture(gAssetManager.LoadTexture(config["advGraphOptFilteringDisabled"].value));
    filteringText->SetReceivesInput(false);

    // Create "trilinear filtering" toggle.
    mTrilinearFilteringToggle = CreateToggle(config, "advGraphOptFiltering", mAdvancedGraphicOptionsSection);
    mTrilinearFilteringToggle->SetToggleCallback([](bool isOn) {
        gRenderer.SetUseTrilinearFiltering(isOn);
    });

    // Create "lod" text (text can be "disabled", like a button, if graphics system doesn't support this option).
    UIButton* lodText = CreateButton(config, "advGraphOptLODText", mAdvancedGraphicOptionsSection, false);
    lodText->SetUpTexture(gAssetManager.LoadTexture(config["advGraphOptLODEnabled"].value));
    lodText->SetDisabledTexture(gAssetManager.LoadTexture(config["advGraphOptLODDisabled"].value));
    lodText->SetReceivesInput(false);

    //TODO: LOD Slider
}

void OptionBar::CreateGameOptionsSection(std::unordered_map<std::string, IniKeyValue>& config)
{
    // Add background image.
    UIImage* backgroundImage = UI::CreateWidgetActor<UIImage>("GameOptionsSection", mAdvancedOptionsSection);
    backgroundImage->SetTexture(gAssetManager.LoadTexture(config["gameOptBackSprite"].value), true);
    mGameOptionsSection = backgroundImage->GetOwner();

    // Position directly below main section.
    backgroundImage->GetRectTransform()->SetAnchor(0.0f, 0.0f);
    backgroundImage->GetRectTransform()->SetPivot(0.0f, 1.0f);
    backgroundImage->SetReceivesInput(true);

    // Create glide camera toggle.
    mCameraGlideToggle = CreateToggle(config, "gameOptGlide", mGameOptionsSection);
    mCameraGlideToggle->SetToggleCallback([](bool isOn){
        GameCamera::SetCameraGlideEnabled(isOn);
    });

    // Create captions toggle.
    mCaptionsToggle = CreateToggle(config, "gameOptCaptions", mGameOptionsSection);
    mCaptionsToggle->SetToggleCallback([](bool isOn) {
        CaptionsOverlay::SetCaptionsEnabled(isOn);
    });

    // Create keyboard controls button.
    UIButton* controlsButton = CreateButton(config, "gameOptControls", mGameOptionsSection);
    controlsButton->SetPressCallback([](UIButton* button) {
        std::cout << "Controls" << std::endl;
    });
}

void OptionBar::OnRadioButtonPressed()
{
    Hide();
    gActionManager.ExecuteSheepAction(StringUtil::Format("wait CallSheep(\"%s\", \"RadioButton$\")", gLocationManager.GetLocation().c_str()));
}

void OptionBar::OnCinematicsButtonPressed(UIButton* button)
{
    if(button == mCinematicsOffButton)
    {
        GameCamera::SetCinematicsEnabled(false);
    }
    else if(button == mCinematicsOnButton)
    {
        GameCamera::SetCinematicsEnabled(true);
    }
    RefreshCinematicsButtonState();
}

void OptionBar::RefreshCinematicsButtonState()
{
    bool cinematicsEnabled = GameCamera::AreCinematicsEnabled();
    mCinematicsOffButton->SetEnabled(cinematicsEnabled);
    mCinematicsOnButton->SetEnabled(!cinematicsEnabled);
}

void OptionBar::OnSoundOptionsButtonPressed(UIButton* button)
{
    // Toggle UI elements and keep on-screen if size expands.
    mSoundOptionsSection->SetActive(!mSoundOptionsSection->IsActive());
    mGraphicOptionsSection->SetActive(false);
    mGameOptionsSection->SetActive(false);

    // If active, refresh options in this section.
    if(mSoundOptionsSection->IsActive())
    {
        // Set toggles based on audio system state.
        // NOTE: the SFX toggle affects both SFX/Ambient audio types...BUT the initial state is only based on SFX state for...idk reasons.
        mGlobalMuteToggle->SetValue(gAudioManager.GetMuted());
        mSfxMuteToggle->SetValue(gAudioManager.GetMuted(AudioType::SFX));
        mVoMuteToggle->SetValue(gAudioManager.GetMuted(AudioType::VO));
        mMusicMuteToggle->SetValue(gAudioManager.GetMuted(AudioType::Music));

        // Set sliders based on audio system state.
        // As with the toggles, SFX slider controls both SFX & Ambient volumes. However, the initial value is set from just SFX.
        // I imagine that Ambient is internally a separate audio type, but to the end user, it is just treated as more SFX.
        mGlobalVolumeSliderSecondary->SetValueSilently(gAudioManager.GetMasterVolume());
        mSfxVolumeSlider->SetValueSilently(gAudioManager.GetVolume(AudioType::SFX));
        mVoVolumeSlider->SetValueSilently(gAudioManager.GetVolume(AudioType::VO));
        mMusicVolumeSlider->SetValueSilently(gAudioManager.GetVolume(AudioType::Music));
    }
}

void OptionBar::OnGlobalVolumeSliderValueChanged(float value)
{
    // Change the volume.
    gAudioManager.SetMasterVolume(value);

    // Since there are two (!!) sliders for global volume, make sure they stay in-sync.
    mGlobalVolumeSlider->SetValueSilently(value);
    mGlobalVolumeSliderSecondary->SetValueSilently(value);
}

void OptionBar::OnGraphicsOptionsButtonPressed(UIButton* button)
{
    mSoundOptionsSection->SetActive(false);
    mGraphicOptionsSection->SetActive(!this->mGraphicOptionsSection->IsActive());
    mGameOptionsSection->SetActive(false);

    // Repopulate resolutions dropdown choices.
    // These could change if you move the game window to a new display.
    std::vector<std::string> resolutionStrings;
    for(auto& resolution : Window::GetResolutions())
    {
        resolutionStrings.push_back(StringUtil::Format("%u x %u", resolution.width, resolution.height));
    }
    mResolutionDropdown->SetChoices(resolutionStrings);

    // Set current resolution in dropdown as the current choice.
    uint32_t currentWidth = Window::GetWidth();
    uint32_t currentHeight = Window::GetHeight();
    mResolutionDropdown->SetCurrentChoice(StringUtil::Format("%u x %u", currentWidth, currentHeight));
}

void OptionBar::OnGameOptionsButtonPressed(UIButton* button)
{
    mSoundOptionsSection->SetActive(false);
    mGraphicOptionsSection->SetActive(false);
    mGameOptionsSection->SetActive(!this->mGameOptionsSection->IsActive());

    // Make sure toggles reflect the current preferences.
    mCameraGlideToggle->SetValue(GameCamera::IsCameraGlideEnabled());
    mCaptionsToggle->SetValue(CaptionsOverlay::CaptionsEnabled());
}