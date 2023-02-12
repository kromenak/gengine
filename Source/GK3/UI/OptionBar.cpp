#include "OptionBar.h"

#include "CaptionsOverlay.h"
#include "GameCamera.h"
#include "GameProgress.h"
#include "InventoryManager.h"
#include "Services.h"
#include "TextAsset.h"
#include "UIButton.h"
#include "UICanvas.h"
#include "UIDrag.h"
#include "UIDropdown.h"
#include "UIImage.h"
#include "UILabel.h"
#include "UISlider.h"
#include "UIToggle.h"
#include "Window.h"

OptionBar::OptionBar() : Actor(TransformType::RectTransform)
{
    // Load layout text file, parse to key/value map, and then delete it.
    TextAsset* optionBarText = Services::GetAssets()->LoadText("RC_LAYOUT.TXT");
    
    IniParser parser(optionBarText->GetText(), optionBarText->GetTextLength());
    parser.SetMultipleKeyValuePairsPerLine(false);
    std::unordered_map<std::string, IniKeyValue> optionBarConfig = parser.ParseAllAsMap();
    
    // Create canvas, to contain the UI components.
    AddComponent<UICanvas>(6);
    
    // Canvas rect fills the entire screen.
    RectTransform* rectTransform = GetComponent<RectTransform>();
    rectTransform->SetSizeDelta(0.0f, 0.0f);
    rectTransform->SetAnchorMin(Vector2::Zero);
    rectTransform->SetAnchorMax(Vector2::One);
    
    // The background of the action bar consists of a fullscreen clickable button area.
    // This stops interaction with the scene while action bar is visible.
    mSceneBlockerButton = AddComponent<UIButton>();
    mSceneBlockerButton->SetPressCallback([this](UIButton* button) {
        Hide();
    });
    
    // Create sections.
    CreateMainSection(optionBarConfig);
    CreateCamerasSection(optionBarConfig);
    CreateOptionsSection(optionBarConfig);
    
    // Hide by default.
    Hide();
}

void OptionBar::Show()
{
    SetActive(true);

    // Only show the "active inventory item" button if there is an active inventory item.
    std::string activeInvItem = Services::Get<InventoryManager>()->GetActiveInventoryItem();
    mActiveInventoryItemButton->SetEnabled(!activeInvItem.empty());
    if(!activeInvItem.empty())
    {
        mActiveInventoryItemButton->SetUpTexture(Services::Get<InventoryManager>()->GetInventoryItemIconTexture(activeInvItem));
    }

    // Update score label.
    mScoreLabel->SetText(StringUtil::Format("%03i/%03i",
                                            Services::Get<GameProgress>()->GetScore(),
                                            Services::Get<GameProgress>()->GetMaxScore()));

    // Position option bar over mouse.
    // "Minus half size" because option bar's pivot is lower-left corner, but want mouse at center.
    // Also, round the "half size" to ensure the UI renders "pixel perfect" - on exact pixel spots rather than between them.
    Vector2 halfSize = mOptionBarRoot->GetSize() * 0.5f;
    halfSize.x = Math::Round(halfSize.x);
    halfSize.y = Math::Round(halfSize.y);
    mOptionBarRoot->SetAnchoredPosition(Services::GetInput()->GetMousePosition() - halfSize);
    
    // Force to be fully on screen.
    KeepOnScreen();
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
}

void OptionBar::OnUpdate(float deltaTime)
{
    // Set buttons interactive only if an action is not playing.
    bool actionActive = Services::Get<ActionManager>()->IsActionPlaying();
    mActiveInventoryItemButton->SetCanInteract(!actionActive);
    mInventoryButton->SetCanInteract(!actionActive);
    mHintButton->SetCanInteract(!actionActive); //TODO: also base this on whether a hint is currently available...
    mCamerasButton->SetCanInteract(!actionActive);
    mHelpButton->SetCanInteract(!actionActive);
    mOptionsButton->SetCanInteract(!actionActive);
    // Note: Cinematics and Close button are always interactive.
}

void OptionBar::KeepOnScreen()
{
    // Make sure the options bar stays on-screen.
    mOptionBarRoot->MoveInsideRect(Window::GetRect());
}

UIButton* CreateButton(std::unordered_map<std::string, IniKeyValue>& config, const std::string& buttonId, Actor* parent, bool setSprites = true)
{
    Actor* buttonActor = new Actor(TransformType::RectTransform);
    buttonActor->GetTransform()->SetParent(parent->GetTransform());
    UIButton* button = buttonActor->AddComponent<UIButton>();
    button->GetRectTransform()->SetAnchor(0.0f, 1.0f);
    button->GetRectTransform()->SetPivot(0.0f, 1.0f);
    
    Vector2 buttonPos = config[buttonId + "Pos"].GetValueAsVector2();
    buttonPos.y *= -1;
    button->GetRectTransform()->SetAnchoredPosition(buttonPos);
    
    if(setSprites)
    {
        auto it = config.find(buttonId + "SpriteUp");
        if(it != config.end())
        {
            button->SetUpTexture(Services::GetAssets()->LoadTexture(it->second.value));
        }
        
        it = config.find(buttonId + "SpriteDown");
        if(it != config.end())
        {
            button->SetDownTexture(Services::GetAssets()->LoadTexture(it->second.value));
        }
        
        it = config.find(buttonId + "SpriteDis");
        if(it != config.end())
        {
            button->SetDisabledTexture(Services::GetAssets()->LoadTexture(it->second.value));
        }
        
        it = config.find(buttonId + "SpriteHov");
        if(it != config.end())
        {
            button->SetHoverTexture(Services::GetAssets()->LoadTexture(it->second.value));
        }
    }
    return button;
}

UIToggle* CreateToggle(std::unordered_map<std::string, IniKeyValue>& config, const std::string& toggleId, Actor* parent)
{
    Actor* toggleActor = new Actor(TransformType::RectTransform);
    toggleActor->GetTransform()->SetParent(parent->GetTransform());

    UIToggle* toggle = toggleActor->AddComponent<UIToggle>();
    toggle->GetRectTransform()->SetAnchor(0.0f, 1.0f);
    toggle->GetRectTransform()->SetPivot(0.0f, 1.0f);
    
    Vector2 togglePos = config[toggleId + "Pos"].GetValueAsVector2();
    togglePos.y *= -1;
    toggle->GetRectTransform()->SetAnchoredPosition(togglePos);
    
    toggle->SetOnTexture(Services::GetAssets()->LoadTexture(config[toggleId + "SpriteDown"].value));
    toggle->SetOffTexture(Services::GetAssets()->LoadTexture(config[toggleId + "SpriteUp"].value));
    toggle->SetDisabledTexture(Services::GetAssets()->LoadTexture(config[toggleId + "SpriteDis"].value));
    return toggle;
}

UISlider* CreateSlider(std::unordered_map<std::string, IniKeyValue>& config, const std::string& sliderId, Actor* parent)
{
    // Create slider.
    Actor* sliderActor = new Actor(TransformType::RectTransform);
    sliderActor->GetTransform()->SetParent(parent->GetTransform());
    
    UISlider* slider = sliderActor->AddComponent<UISlider>();
    slider->GetRectTransform()->SetAnchor(0.0f, 1.0f);
    slider->GetRectTransform()->SetPivot(0.0f, 1.0f);

    // Set slider min/max positions.
    // This makes the slider's rect transform the exact width required for the min/max slider positions.
    Vector2 minPos = config[sliderId + "Min"].GetValueAsVector2();
    minPos.y *= -1;
    slider->GetRectTransform()->SetAnchoredPosition(minPos);

    Vector2 maxPos = config[sliderId + "Max"].GetValueAsVector2();
    maxPos.y *= -1;

    // Create slider handle actor.
    Actor* handleActor = new Actor(TransformType::RectTransform);
    slider->SetHandleActor(handleActor);

    // Add handle image.
    UIImage* handleImage = handleActor->AddComponent<UIImage>();
    handleImage->SetTexture(Services::GetAssets()->LoadTexture("RC_SO_SLIDER"), true);
    
    // Set slider's vertical size to be exactly equal to handle size.
    // This should make it so the handle can only move left/right.
    Vector2 handleSize = handleImage->GetRectTransform()->GetSize();
    slider->GetRectTransform()->SetSizeDeltaY(handleSize.y);

    // Slider's horizontal size is slider width, PLUS width of the handle itself.
    float sliderWidth = (maxPos.x - minPos.x);
    slider->GetRectTransform()->SetSizeDeltaX(sliderWidth + handleSize.x);
    return slider;
}

void OptionBar::CreateMainSection(std::unordered_map<std::string, IniKeyValue>& config)
{
    // Create "root" actor for action bar.
    Actor* optionBar = new Actor(TransformType::RectTransform);
    mOptionBarRoot = optionBar->GetComponent<RectTransform>();
    mOptionBarRoot->SetParent(GetTransform());
    
    // Add background image.
    UIImage* backgroundImage = optionBar->AddComponent<UIImage>();
    backgroundImage->SetTexture(Services::GetAssets()->LoadTexture(config["backSprite"].value), true);
    
    // Some positioning code would be easier if this was (0.5, 0.5) BUT that causes some rendering problems b/c the rect is no longer pixel perfect.
    // Mayyybe we can fix that in the RectTransform/Rect code directly...with a bit of work.
    backgroundImage->GetRectTransform()->SetPivot(Vector2::Zero);
    backgroundImage->GetRectTransform()->SetAnchor(Vector2::Zero);
    backgroundImage->GetRectTransform()->SetAnchoredPosition(Vector2::Zero);

    // The background is draggable.
    UIDrag* drag = optionBar->AddComponent<UIDrag>();
    drag->SetBoundaryRectTransform(GetComponent<RectTransform>());
    drag->SetUseHighlightCursor(false); // but don't highlight cursor when hovering it

    // Load font.
    Font* font = Services::GetAssets()->LoadFont(config["statusFont"].value);
    
    // Add score text.
    Actor* scoreActor = new Actor(TransformType::RectTransform);
    scoreActor->GetTransform()->SetParent(mOptionBarRoot);
    mScoreLabel = scoreActor->AddComponent<UILabel>();
    
    mScoreLabel->GetRectTransform()->SetSizeDelta(config["scoreSize"].GetValueAsVector2());
    mScoreLabel->GetRectTransform()->SetAnchor(0.0f, 1.0f);
    mScoreLabel->GetRectTransform()->SetPivot(0.0f, 1.0f);
    
    Vector2 scorePos = config["scorePos"].GetValueAsVector2();
    scorePos.y *= -1;
    mScoreLabel->GetRectTransform()->SetAnchoredPosition(scorePos);
    
    mScoreLabel->SetFont(font);
    mScoreLabel->SetHorizonalAlignment(HorizontalAlignment::Center);
    mScoreLabel->SetVerticalAlignment(VerticalAlignment::Top);
    mScoreLabel->SetMasked(true);

    // Determine display strings for the day/time.
    // We only have to do this on creation (and not show) because the day/time never change in the middle of a scene.
    std::vector<std::string> dayAndTimeStrings = StringUtil::Split(Services::Get<GameProgress>()->GetTimeblockDisplayName(), ',');
    StringUtil::Trim(dayAndTimeStrings[0]);
    StringUtil::Trim(dayAndTimeStrings[1]);

    // Add day text.
    Actor* dayActor = new Actor(TransformType::RectTransform);
    dayActor->GetTransform()->SetParent(mOptionBarRoot);
    UILabel* dayLabel = dayActor->AddComponent<UILabel>();
    
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
    Actor* timeActor = new Actor(TransformType::RectTransform);
    timeActor->GetTransform()->SetParent(mOptionBarRoot);
    UILabel* timeLabel = timeActor->AddComponent<UILabel>();
    
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
    mActiveInventoryItemButton = CreateButton(config, "currInv", optionBar);
    mActiveInventoryItemButton->SetPressCallback([this](UIButton* button) {
        Hide();
        Services::Get<InventoryManager>()->InventoryInspect();
    });
    
    // Add inventory button.
    mInventoryButton = CreateButton(config, "closed", optionBar);
    mInventoryButton->SetPressCallback([this](UIButton* button) {
        Hide();
        Services::Get<InventoryManager>()->ShowInventory();
    });
    
    // Add hint button.
    mHintButton = CreateButton(config, "hint", optionBar);
    mHintButton->SetPressCallback([](UIButton* button) {
        std::cout << "Hint!" << std::endl;
    });
    
    // Add camera button.
    mCamerasButton = CreateButton(config, "camera", optionBar);
    mCamerasButton->SetPressCallback([this](UIButton* button) {
        mCamerasSection->SetActive(!mCamerasSection->IsActive());
        mOptionsSection->SetActive(false);
        KeepOnScreen();
    });
    
    // Add cinematics button.
    UIButton* cinematicsOnButton = CreateButton(config, "cine", optionBar);
    cinematicsOnButton->SetPressCallback([](UIButton* button) {
        std::cout << "Turn cinematics on!" << std::endl;
    });
    UIButton* cinematicsOffButton = CreateButton(config, "cineoff", optionBar);
    cinematicsOffButton->SetPressCallback([](UIButton* button) {
        std::cout << "Turn cinematics off!" << std::endl;
    });
    
    // Add help button.
    mHelpButton = CreateButton(config, "help", optionBar);
    mHelpButton->SetPressCallback([](UIButton* button) {
        std::cout << "Help!" << std::endl;
    });
    
    // Add options button.
    mOptionsButton = CreateButton(config, "options", optionBar);
    mOptionsButton->SetPressCallback([this](UIButton* button) {
        mCamerasSection->SetActive(false);
        mOptionsSection->SetActive(!mOptionsSection->IsActive());
        KeepOnScreen();
        mGlobalVolumeSlider->SetValueSilently(Services::GetAudio()->GetMasterVolume());
    });
    
    // Add close button.
    UIButton* closeButton = CreateButton(config, "exit", optionBar);
    closeButton->SetPressCallback([this](UIButton* button) {
        Hide();
    });
}

void OptionBar::CreateCamerasSection(std::unordered_map<std::string, IniKeyValue>& config)
{
    // Make this a child of the "main section" root.
    mCamerasSection = new Actor(TransformType::RectTransform);
    mCamerasSection->GetTransform()->SetParent(mOptionBarRoot);
    
    // Add background image.
    UIImage* backgroundImage = mCamerasSection->AddComponent<UIImage>();
    backgroundImage->SetTexture(Services::GetAssets()->LoadTexture(config["camBackSprite"].value), true);
    
    // Position directly below main section.
    backgroundImage->GetRectTransform()->SetAnchor(0.0f, 0.0f);
    backgroundImage->GetRectTransform()->SetPivot(0.0f, 1.0f);
    backgroundImage->GetRectTransform()->SetAnchoredPosition(Vector2::Zero);
    backgroundImage->SetReceivesInput(true);
}

void OptionBar::CreateOptionsSection(std::unordered_map<std::string, IniKeyValue>& config)
{
    // Make this a child of the "main section" root.
    mOptionsSection = new Actor(TransformType::RectTransform);
    mOptionsSection->GetTransform()->SetParent(mOptionBarRoot);
    
    // Add background image.
    UIImage* backgroundImage = mOptionsSection->AddComponent<UIImage>();
    backgroundImage->SetTexture(Services::GetAssets()->LoadTexture(config["optBackSprite"].value), true);
    
    // Position directly below main section.
    backgroundImage->GetRectTransform()->SetAnchor(0.0f, 0.0f);
    backgroundImage->GetRectTransform()->SetPivot(0.0f, 1.0f);
    backgroundImage->GetRectTransform()->SetAnchoredPosition(Vector2::Zero);
    backgroundImage->SetReceivesInput(true);
    
    // Create volume slider.
    mGlobalVolumeSlider = CreateSlider(config, "optSlider", mOptionsSection);
    mGlobalVolumeSlider->SetValueChangeCallback(std::bind(&OptionBar::OnGlobalVolumeSliderValueChanged, this, std::placeholders::_1));
    
    // Create save button.
    UIButton* saveButton = CreateButton(config, "optSave", mOptionsSection);
    saveButton->SetPressCallback([](UIButton* button) {
        std::cout << "Save!" << std::endl;
    });
    
    // Create restore button.
    UIButton* restoreButton = CreateButton(config, "optRestore", mOptionsSection);
    restoreButton->SetPressCallback([](UIButton* button) {
        std::cout << "Restore!" << std::endl;
    });
    
    // Created "advanced options" button.
    UIButton* advOptButton = CreateButton(config, "optAdvanced", mOptionsSection);
    advOptButton->SetPressCallback([this](UIButton* button) {
        this->mAdvancedOptionsSection->SetActive(!this->mAdvancedOptionsSection->IsActive());
        this->KeepOnScreen();
    });
    
    // Create "quit game" button.
    UIButton* quitButton = CreateButton(config, "optQuit", mOptionsSection);
    quitButton->SetPressCallback([this](UIButton* button) {
        this->Hide();
        //TODO: Show "are you sure you want to quit?" dialog.
        GEngine::Instance()->Quit();
    });
    
    // Create advanced options section.
    CreateAdvancedOptionsSection(config);
}

void OptionBar::CreateAdvancedOptionsSection(std::unordered_map<std::string, IniKeyValue>& config)
{
    // Make this a child of the "options section" root.
    mAdvancedOptionsSection = new Actor(TransformType::RectTransform);
    mAdvancedOptionsSection->GetTransform()->SetParent(mOptionsSection->GetTransform());
    
    // Add background image.
    UIImage* backgroundImage = mAdvancedOptionsSection->AddComponent<UIImage>();
    backgroundImage->SetTexture(Services::GetAssets()->LoadTexture(config["advOptBackSprite"].value), true);
    
    // Position directly below main section.
    backgroundImage->GetRectTransform()->SetAnchor(0.0f, 0.0f);
    backgroundImage->GetRectTransform()->SetPivot(0.0f, 1.0f);
    backgroundImage->GetRectTransform()->SetAnchoredPosition(Vector2::Zero);
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
    // Make this a child of the "options section" root.
    mSoundOptionsSection = new Actor(TransformType::RectTransform);
    mSoundOptionsSection->GetTransform()->SetParent(mAdvancedOptionsSection->GetTransform());
    
    // Add background image.
    UIImage* backgroundImage = mSoundOptionsSection->AddComponent<UIImage>();
    backgroundImage->SetTexture(Services::GetAssets()->LoadTexture(config["soundOptBackSprite"].value), true);
    
    // Position directly below main section.
    backgroundImage->GetRectTransform()->SetAnchor(0.0f, 0.0f);
    backgroundImage->GetRectTransform()->SetPivot(0.0f, 1.0f);
    backgroundImage->GetRectTransform()->SetAnchoredPosition(Vector2::Zero);
    backgroundImage->SetReceivesInput(true);

    // Create global volume slider.
    mGlobalVolumeSliderSecondary = CreateSlider(config, "optSoundGlobalSlider", mSoundOptionsSection);
    mGlobalVolumeSliderSecondary->SetValueChangeCallback(std::bind(&OptionBar::OnGlobalVolumeSliderValueChanged, this, std::placeholders::_1));

    // Create global mute toggle.
    mGlobalMuteToggle = CreateToggle(config, "soundOptGlobal", mSoundOptionsSection);
    mGlobalMuteToggle->SetToggleCallback([](bool isOn) {
        Services::GetAudio()->SetMuted(isOn);
    });
    
    // Create sfx volume slider.
    mSfxVolumeSlider = CreateSlider(config, "optSoundSfxSlider", mSoundOptionsSection);
    mSfxVolumeSlider->SetValueChangeCallback([](float value) {
        // The SFX volume slider also controls ambient volume.
        Services::GetAudio()->SetVolume(AudioType::SFX, value);
        Services::GetAudio()->SetVolume(AudioType::Ambient, value);
    });

    //Create sfx mute toggle.
    mSfxMuteToggle = CreateToggle(config, "soundOptSfx", mSoundOptionsSection);
    mSfxMuteToggle->SetToggleCallback([](bool isOn) {
        // The SFX toggle also controls ambient.
        Services::GetAudio()->SetMuted(AudioType::SFX, isOn);
        Services::GetAudio()->SetMuted(AudioType::Ambient, isOn);
    });
    
    // Create VO volume slider.
    mVoVolumeSlider = CreateSlider(config, "optSoundDialogueSlider", mSoundOptionsSection);
    mVoVolumeSlider->SetValueChangeCallback([](float value) {
        Services::GetAudio()->SetVolume(AudioType::VO, value);
    });

    // Create VO mute toggle.
    mVoMuteToggle = CreateToggle(config, "soundOptDialogue", mSoundOptionsSection);
    mVoMuteToggle->SetToggleCallback([](bool isOn) {
        Services::GetAudio()->SetMuted(AudioType::VO, isOn);
    });
    
    // Create music volume slider.
    mMusicVolumeSlider = CreateSlider(config, "optSoundMusicSlider", mSoundOptionsSection);
    mMusicVolumeSlider->SetValueChangeCallback([](float value) {
        Services::GetAudio()->SetVolume(AudioType::Music, value);
    });

    // Create music mute toggle.
    mMusicMuteToggle = CreateToggle(config, "soundOptMusic", mSoundOptionsSection);
    mMusicMuteToggle->SetToggleCallback([](bool isOn) {
        Services::GetAudio()->SetMuted(AudioType::Music, isOn);
    });
    
    // Create max sounds slider.
    UISlider* maxSoundsSlider = CreateSlider(config, "optSoundNumSlider", mSoundOptionsSection);
    maxSoundsSlider->SetValueChangeCallback([](float value) {
        std::cout << "Changed max sounds to " << value << std::endl;
    });
}

void OptionBar::CreateGraphicOptionsSection(std::unordered_map<std::string, IniKeyValue>& config)
{
    // Make this a child of the "options section" root.
    mGraphicOptionsSection = new Actor(TransformType::RectTransform);
    mGraphicOptionsSection->GetTransform()->SetParent(mAdvancedOptionsSection->GetTransform());
    
    // Add background image.
    UIImage* backgroundImage = mGraphicOptionsSection->AddComponent<UIImage>();
    backgroundImage->SetTexture(Services::GetAssets()->LoadTexture(config["graphicsOptBackSprite"].value), true);
    
    // Position directly below main section.
    backgroundImage->GetRectTransform()->SetAnchor(0.0f, 0.0f);
    backgroundImage->GetRectTransform()->SetPivot(0.0f, 1.0f);
    backgroundImage->GetRectTransform()->SetAnchoredPosition(Vector2::Zero);
    backgroundImage->SetReceivesInput(true);
    
    //TODO: Gamma slider
    
    //TODO: 3D driver dropdown
    
    // Create "incremental rendering" text (text can be "disabled", like a button, if graphics system doesn't support this option).
    UIButton* incRenderingText = CreateButton(config, "graphOptIncrementalText", mGraphicOptionsSection, false);
    incRenderingText->SetUpTexture(Services::GetAssets()->LoadTexture(config["graphOptIncrementalEnabled"].value));
    incRenderingText->SetDisabledTexture(Services::GetAssets()->LoadTexture(config["graphOptIncrementalDisabled"].value));
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
        KeepOnScreen();

        if(mAdvancedGraphicOptionsSection->IsActive())
        {
            mMipmapsToggle->SetValue(Services::GetRenderer()->UseMipmaps());
            mTrilinearFilteringToggle->SetValue(Services::GetRenderer()->UseTrilinearFiltering());
        }
    });
    
    CreateAdvancedGraphicOptionsSection(config);

    // Resolution dropdown (create after other elements so dropdown box draws above everything).
    mResolutionDropdown = new UIDropdown(this);
    mResolutionDropdown->SetMaxVisibleChoices(5);
    RectTransform* resolutionDropdownRT = mResolutionDropdown->GetComponent<RectTransform>();

    resolutionDropdownRT->SetParent(mGraphicOptionsSection->GetTransform());
    resolutionDropdownRT->SetAnchor(0.0f, 1.0f);
    resolutionDropdownRT->SetPivot(0.0f, 1.0f);

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
            Services::GetRenderer()->ChangeResolution(resolutions[selectedIndex]);
        }
    });
}

void OptionBar::CreateAdvancedGraphicOptionsSection(std::unordered_map<std::string, IniKeyValue>& config)
{
    // Make this a child of the "options section" root.
    mAdvancedGraphicOptionsSection = new Actor(TransformType::RectTransform);
    mAdvancedGraphicOptionsSection->GetTransform()->SetParent(mGraphicOptionsSection->GetTransform());
    
    // Add background image.
    UIImage* backgroundImage = mAdvancedGraphicOptionsSection->AddComponent<UIImage>();
    backgroundImage->SetTexture(Services::GetAssets()->LoadTexture(config["advGraphOptBackSprite"].value), true);
    
    // Position directly below main section.
    backgroundImage->GetRectTransform()->SetAnchor(0.0f, 0.0f);
    backgroundImage->GetRectTransform()->SetPivot(0.0f, 1.0f);
    backgroundImage->GetRectTransform()->SetAnchoredPosition(Vector2::Zero);
    backgroundImage->SetReceivesInput(true);
    
    //TODO: Texture quality dropdown
    //TODO: Lighting quality dropdown
    
    // Create "mipmap" text (text can be "disabled", like a button, if graphics system doesn't support this option).
    UIButton* mipmapText = CreateButton(config, "advGraphOptMipMapText", mAdvancedGraphicOptionsSection, false);
    mipmapText->SetUpTexture(Services::GetAssets()->LoadTexture(config["advGraphOptMipMapEnabled"].value));
    mipmapText->SetDisabledTexture(Services::GetAssets()->LoadTexture(config["advGraphOptMipMapDisabled"].value));
    mipmapText->SetReceivesInput(false);
    
    // Create "mipmap" toggle.
    mMipmapsToggle = CreateToggle(config, "advGraphOptMipMap", mAdvancedGraphicOptionsSection);
    mMipmapsToggle->SetToggleCallback([](bool isOn) {
        Services::GetRenderer()->SetUseMipmaps(isOn);
    });
    
    // Create "animation interpolation" text (text can be "disabled", like a button, if graphics system doesn't support this option).
    UIButton* animInterpText = CreateButton(config, "advGraphOptInterpolationText", mAdvancedGraphicOptionsSection, false);
    animInterpText->SetUpTexture(Services::GetAssets()->LoadTexture(config["advGraphOptInterpolationEnabled"].value));
    animInterpText->SetDisabledTexture(Services::GetAssets()->LoadTexture(config["advGraphOptInterpolationDisabled"].value));
    animInterpText->SetReceivesInput(false);
    
    // Create "animation interpolation" toggle.
    UIToggle* animInterpToggle = CreateToggle(config, "advGraphOptInterpolation", mAdvancedGraphicOptionsSection);
    animInterpToggle->SetToggleCallback([](bool isOn) {
        std::cout << "Anim interp is toggled " << isOn << std::endl;
    });
    
    // Create "trilinear filtering" text (text can be "disabled", like a button, if graphics system doesn't support this option).
    UIButton* filteringText = CreateButton(config, "advGraphOptFilteringText", mAdvancedGraphicOptionsSection, false);
    filteringText->SetUpTexture(Services::GetAssets()->LoadTexture(config["advGraphOptFilteringEnabled"].value));
    filteringText->SetDisabledTexture(Services::GetAssets()->LoadTexture(config["advGraphOptFilteringDisabled"].value));
    filteringText->SetReceivesInput(false);
    
    // Create "trilinear filtering" toggle.
    mTrilinearFilteringToggle = CreateToggle(config, "advGraphOptFiltering", mAdvancedGraphicOptionsSection);
    mTrilinearFilteringToggle->SetToggleCallback([](bool isOn) {
        Services::GetRenderer()->SetUseTrilinearFiltering(isOn);
    });
    
    // Create "lod" text (text can be "disabled", like a button, if graphics system doesn't support this option).
    UIButton* lodText = CreateButton(config, "advGraphOptLODText", mAdvancedGraphicOptionsSection, false);
    lodText->SetUpTexture(Services::GetAssets()->LoadTexture(config["advGraphOptLODEnabled"].value));
    lodText->SetDisabledTexture(Services::GetAssets()->LoadTexture(config["advGraphOptLODDisabled"].value));
    lodText->SetReceivesInput(false);
    
    //TODO: LOD Slider
}

void OptionBar::CreateGameOptionsSection(std::unordered_map<std::string, IniKeyValue>& config)
{
    // Make this a child of the "options section" root.
    mGameOptionsSection = new Actor(TransformType::RectTransform);
    mGameOptionsSection->GetTransform()->SetParent(mAdvancedOptionsSection->GetTransform());
    
    // Add background image.
    UIImage* backgroundImage = mGameOptionsSection->AddComponent<UIImage>();
    backgroundImage->SetTexture(Services::GetAssets()->LoadTexture(config["gameOptBackSprite"].value), true);
    
    // Position directly below main section.
    backgroundImage->GetRectTransform()->SetAnchor(0.0f, 0.0f);
    backgroundImage->GetRectTransform()->SetPivot(0.0f, 1.0f);
    backgroundImage->GetRectTransform()->SetAnchoredPosition(Vector2::Zero);
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
    controlsButton->SetPressCallback([this](UIButton* button) {
        std::cout << "Controls" << std::endl;
    });
}

void OptionBar::OnSoundOptionsButtonPressed(UIButton* button)
{
    // Toggle UI elements and keep on-screen if size expands.
    mSoundOptionsSection->SetActive(!mSoundOptionsSection->IsActive());
    mGraphicOptionsSection->SetActive(false);
    mGameOptionsSection->SetActive(false);
    KeepOnScreen();

    // If active, refresh options in this section.
    if(mSoundOptionsSection->IsActive())
    {
        // Set toggles based on audio system state.
        // NOTE: the SFX toggle affects both SFX/Ambient audio types...BUT the initial state is only based on SFX state for...idk reasons.
        mGlobalMuteToggle->SetValue(Services::GetAudio()->GetMuted());
        mSfxMuteToggle->SetValue(Services::GetAudio()->GetMuted(AudioType::SFX));
        mVoMuteToggle->SetValue(Services::GetAudio()->GetMuted(AudioType::VO));
        mMusicMuteToggle->SetValue(Services::GetAudio()->GetMuted(AudioType::Music));

        // Set sliders based on audio system state.
        // As with the toggles, SFX slider controls both SFX & Ambient volumes. However, the initial value is set from just SFX.
        // I imagine that Ambient is internally a separate audio type, but to the end user, it is just treated as more SFX.
        mGlobalVolumeSliderSecondary->SetValueSilently(Services::GetAudio()->GetMasterVolume());
        mSfxVolumeSlider->SetValueSilently(Services::GetAudio()->GetVolume(AudioType::SFX));
        mVoVolumeSlider->SetValueSilently(Services::GetAudio()->GetVolume(AudioType::VO));
        mMusicVolumeSlider->SetValueSilently(Services::GetAudio()->GetVolume(AudioType::Music));
    }
}

void OptionBar::OnGlobalVolumeSliderValueChanged(float value)
{
    // Change the volume.
    Services::GetAudio()->SetMasterVolume(value);

    // Since there are two (!!) sliders for global volume, make sure they stay in-sync.
    mGlobalVolumeSlider->SetValueSilently(value);
    mGlobalVolumeSliderSecondary->SetValueSilently(value);
}

void OptionBar::OnGraphicsOptionsButtonPressed(UIButton* button)
{
    mSoundOptionsSection->SetActive(false);
    mGraphicOptionsSection->SetActive(!this->mGraphicOptionsSection->IsActive());
    mGameOptionsSection->SetActive(false);
    KeepOnScreen();

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
    KeepOnScreen();

    // Make sure toggles reflect the current preferences.
    mCameraGlideToggle->SetValue(GameCamera::IsCameraGlideEnabled());
    mCaptionsToggle->SetValue(CaptionsOverlay::CaptionsEnabled());
}
