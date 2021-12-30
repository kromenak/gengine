#include "OptionBar.h"

#include "InventoryManager.h"
#include "Services.h"
#include "UIButton.h"
#include "UICanvas.h"
#include "UIDrag.h"
#include "UIImage.h"
#include "UILabel.h"
#include "UISlider.h"
#include "UIToggle.h"

OptionBar::OptionBar() : Actor(Actor::TransformType::RectTransform)
{
    // Load layout text file, parse to key/value map, and then delete it.
    TextAsset* optionBarText = Services::GetAssets()->LoadText("RC_LAYOUT.TXT");
    
    IniParser parser(optionBarText->GetText(), optionBarText->GetTextLength());
    parser.SetMultipleKeyValuePairsPerLine(false);
    std::unordered_map<std::string, IniKeyValue> optionBarConfig = parser.ParseAllAsMap();
    
    // Create canvas, to contain the UI components.
    mCanvas = AddComponent<UICanvas>();
    
    // Canvas rect fills the entire screen.
    RectTransform* rectTransform = mCanvas->GetRectTransform();
    rectTransform->SetSizeDelta(0.0f, 0.0f);
    rectTransform->SetAnchorMin(Vector2::Zero);
    rectTransform->SetAnchorMax(Vector2::One);
    
    // The background of the action bar consists of a fullscreen clickable button area.
    // This stops interaction with the scene while action bar is visible.
    mSceneBlockerButton = AddComponent<UIButton>();
    mCanvas->AddWidget(mSceneBlockerButton);
    mSceneBlockerButton->SetPressCallback([this]() {
        this->Hide();
    });
    
    // Create sections.
    CreateMainSection(mCanvas, optionBarConfig);
    CreateCamerasSection(mCanvas, optionBarConfig);
    CreateOptionsSection(mCanvas, optionBarConfig);
    
    // Hide by default.
    Hide();
}

void OptionBar::Show()
{
    SetActive(true);
    
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
	if(Services::GetInput()->IsKeyLeadingEdge(SDL_SCANCODE_L))
    {
        RectTransform* rt = static_cast<RectTransform*>(GetTransform());
        Vector2 ap = rt->GetAnchoredPosition();
        ap.x += 10;
        ap.y += 10;
        rt->SetAnchoredPosition(ap);
    }
    if(Services::GetInput()->IsKeyLeadingEdge(SDL_SCANCODE_K))
    {
        RectTransform* rt = static_cast<RectTransform*>(GetTransform());
        Vector2 ap = rt->GetAnchoredPosition();
        ap.x -= 10;
        ap.y -= 10;
        rt->SetAnchoredPosition(ap);
    }
}

void OptionBar::KeepOnScreen()
{
    // Make sure the options bar stays on-screen.
    mOptionBarRoot->MoveInsideRect(Services::GetRenderer()->GetScreenRect());
}

UIButton* CreateButton(UICanvas* canvas, std::unordered_map<std::string, IniKeyValue>& config, const std::string& buttonId, Actor* parent, bool setSprites = true)
{
    Actor* buttonActor = new Actor(Actor::TransformType::RectTransform);
    UIButton* button = buttonActor->AddComponent<UIButton>();
    canvas->AddWidget(button);
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

    if(parent != nullptr)
    {
        button->GetRectTransform()->SetParent(parent->GetTransform());
    }
    return button;
}

UIToggle* CreateToggle(UICanvas* canvas, std::unordered_map<std::string, IniKeyValue>& config, const std::string& toggleId, Actor* parent)
{
    Actor* toggleActor = new Actor(Actor::TransformType::RectTransform);
    UIToggle* toggle = toggleActor->AddComponent<UIToggle>();
    canvas->AddWidget(toggle);
    toggle->GetRectTransform()->SetAnchor(0.0f, 1.0f);
    toggle->GetRectTransform()->SetPivot(0.0f, 1.0f);
    
    Vector2 togglePos = config[toggleId + "Pos"].GetValueAsVector2();
    togglePos.y *= -1;
    toggle->GetRectTransform()->SetAnchoredPosition(togglePos);
    
    toggle->SetOnTexture(Services::GetAssets()->LoadTexture(config[toggleId + "SpriteDown"].value));
    toggle->SetOffTexture(Services::GetAssets()->LoadTexture(config[toggleId + "SpriteUp"].value));
    toggle->SetDisabledTexture(Services::GetAssets()->LoadTexture(config[toggleId + "SpriteDis"].value));
    
    if(parent != nullptr)
    {
        toggle->GetRectTransform()->SetParent(parent->GetTransform());
    }
    return toggle;
}

UISlider* CreateSlider(UICanvas* canvas, std::unordered_map<std::string, IniKeyValue>& config, const std::string& sliderId, Actor* parent)
{
    // Create slider.
    Actor* sliderActor = new Actor(Actor::TransformType::RectTransform);
    UISlider* slider = sliderActor->AddComponent<UISlider>();
    canvas->AddWidget(slider);
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
    Actor* handleActor = new Actor(Actor::TransformType::RectTransform);
    UIImage* handleImage = handleActor->AddComponent<UIImage>();
    canvas->AddWidget(handleImage);
    handleImage->SetTextureAndSize(Services::GetAssets()->LoadTexture("RC_SO_SLIDER"));
    
    // Add drag component to the handle (so we can drag it) and tell slider to use this for dragging.
    UIDrag* drag = handleActor->AddComponent<UIDrag>();
    canvas->AddWidget(drag);
    slider->SetHandleActor(handleActor);

    // Set slider's vertical size to be exactly equal to handle size.
    // This should make it so the handle can only move left/right.
    Vector2 handleSize = handleImage->GetRectTransform()->GetSize();
    slider->GetRectTransform()->SetSizeDeltaY(handleSize.y);

    // Slider's horizontal size is slider width, PLUS width of the handle itself.
    float sliderWidth = (maxPos.x - minPos.x);
    slider->GetRectTransform()->SetSizeDeltaX(sliderWidth + handleSize.x);

    // Set slider parent if told to.
    if(parent != nullptr)
    {
        slider->GetRectTransform()->SetParent(parent->GetTransform());
    }
    return slider;
}

void OptionBar::CreateMainSection(UICanvas* canvas, std::unordered_map<std::string, IniKeyValue>& config)
{
    // Create "root" actor for action bar.
    Actor* optionBar = new Actor(Actor::TransformType::RectTransform);
    mOptionBarRoot = optionBar->GetComponent<RectTransform>();
    mOptionBarRoot->SetParent(mCanvas->GetRectTransform());
    
    // Add background image.
    UIImage* backgroundImage = optionBar->AddComponent<UIImage>();
    canvas->AddWidget(backgroundImage);
    backgroundImage->SetTextureAndSize(Services::GetAssets()->LoadTexture(config["backSprite"].value));
    
    // Some positioning code would be easier if this was (0.5, 0.5) BUT that causes some rendering problems b/c the rect is no longer pixel perfect.
    // Mayyybe we can fix that in the RectTransform/Rect code directly...with a bit of work.
    backgroundImage->GetRectTransform()->SetPivot(Vector2::Zero);
    backgroundImage->GetRectTransform()->SetAnchor(Vector2::Zero);
    backgroundImage->GetRectTransform()->SetAnchoredPosition(Vector2::Zero);

    // The background is draggable.
    UIDrag* drag = optionBar->AddComponent<UIDrag>();
    canvas->AddWidget(drag);
    drag->SetBoundaryRect(Services::GetRenderer()->GetScreenRect());
    drag->SetUseHighlightCursor(false); // but don't highlight cursor when hovering it

    // Load font.
    Font* font = Services::GetAssets()->LoadFont(config["statusFont"].value);
    
    // Add score text.
    Actor* scoreActor = new Actor(Actor::TransformType::RectTransform);
    scoreActor->GetTransform()->SetParent(mOptionBarRoot);
    UILabel* scoreLabel = scoreActor->AddComponent<UILabel>();
    canvas->AddWidget(scoreLabel);
    
    scoreLabel->GetRectTransform()->SetSizeDelta(config["scoreSize"].GetValueAsVector2());
    scoreLabel->GetRectTransform()->SetAnchor(0.0f, 1.0f);
    scoreLabel->GetRectTransform()->SetPivot(0.0f, 1.0f);
    
    Vector2 scorePos = config["scorePos"].GetValueAsVector2();
    scorePos.y *= -1;
    scoreLabel->GetRectTransform()->SetAnchoredPosition(scorePos);
    
    scoreLabel->SetFont(font);
    scoreLabel->SetText("000/965");
    scoreLabel->SetHorizonalAlignment(HorizontalAlignment::Center);
    
    // Add day text.
    Actor* dayActor = new Actor(Actor::TransformType::RectTransform);
    dayActor->GetTransform()->SetParent(mOptionBarRoot);
    UILabel* dayLabel = dayActor->AddComponent<UILabel>();
    canvas->AddWidget(dayLabel);
    
    dayLabel->GetRectTransform()->SetSizeDelta(config["daySize"].GetValueAsVector2());
    dayLabel->GetRectTransform()->SetAnchor(0.0f, 1.0f);
    dayLabel->GetRectTransform()->SetPivot(0.0f, 1.0f);
    
    Vector2 dayPos = config["dayPos"].GetValueAsVector2();
    dayPos.y *= -1;
    dayLabel->GetRectTransform()->SetAnchoredPosition(dayPos);
    
    dayLabel->SetFont(font);
    dayLabel->SetText("Day 1");
    dayLabel->SetHorizonalAlignment(HorizontalAlignment::Center);
    
    // Add time text.
    Actor* timeActor = new Actor(Actor::TransformType::RectTransform);
    timeActor->GetTransform()->SetParent(mOptionBarRoot);
    UILabel* timeLabel = timeActor->AddComponent<UILabel>();
    canvas->AddWidget(timeLabel);
    
    timeLabel->GetRectTransform()->SetSizeDelta(config["timeSize"].GetValueAsVector2());
    timeLabel->GetRectTransform()->SetAnchor(0.0f, 1.0f);
    timeLabel->GetRectTransform()->SetPivot(0.0f, 1.0f);
    
    Vector2 timePos = config["timePos"].GetValueAsVector2();
    timePos.y *= -1;
    timeLabel->GetRectTransform()->SetAnchoredPosition(timePos);
    
    timeLabel->SetFont(font);
    timeLabel->SetText("10am - 12pm");
    timeLabel->SetHorizonalAlignment(HorizontalAlignment::Center);
    
    //TODO: Active inv item
    
    // Add inventory button.
    UIButton* invButton = CreateButton(canvas, config, "closed", optionBar);
    invButton->SetPressCallback([this]() {
        this->Hide();
        Services::Get<InventoryManager>()->ShowInventory();
    });
    
    // Add hint button.
    UIButton* hintButton = CreateButton(canvas, config, "hint", optionBar);
    hintButton->SetPressCallback([]() {
        std::cout << "Hint!" << std::endl;
    });
    
    // Add camera button.
    UIButton* cameraButton = CreateButton(canvas, config, "camera", optionBar);
    cameraButton->SetPressCallback([this]() {
        this->mCamerasSection->SetActive(!this->mCamerasSection->IsActive());
        this->mOptionsSection->SetActive(false);
        this->KeepOnScreen();
    });
    
    // Add cinematics button.
    UIButton* cinematicsOnButton = CreateButton(canvas, config, "cine", optionBar);
    cinematicsOnButton->SetPressCallback([]() {
        std::cout << "Turn cinematics on!" << std::endl;
    });
    UIButton* cinematicsOffButton = CreateButton(canvas, config, "cineoff", optionBar);
    cinematicsOffButton->SetPressCallback([]() {
        std::cout << "Turn cinematics off!" << std::endl;
    });
    
    // Add help button.
    UIButton* helpButton = CreateButton(canvas, config, "help", optionBar);
    helpButton->SetPressCallback([]() {
        std::cout << "Help!" << std::endl;
    });
    
    // Add options button.
    UIButton* optionsButton = CreateButton(canvas, config, "options", optionBar);
    optionsButton->SetPressCallback([this]() {
        mCamerasSection->SetActive(false);
        mOptionsSection->SetActive(!mOptionsSection->IsActive());
        KeepOnScreen();
        mGlobalVolumeSlider->SetValueSilently(Services::GetAudio()->GetMasterVolume());
    });
    
    // Add close button.
    UIButton* closeButton = CreateButton(canvas, config, "exit", optionBar);
    closeButton->SetPressCallback([this]() {
        this->Hide();
    });
}

void OptionBar::CreateCamerasSection(UICanvas* canvas, std::unordered_map<std::string, IniKeyValue>& config)
{
    // Make this a child of the "main section" root.
    mCamerasSection = new Actor(Actor::TransformType::RectTransform);
    mCamerasSection->GetTransform()->SetParent(mOptionBarRoot);
    
    // Add background image.
    UIImage* backgroundImage = mCamerasSection->AddComponent<UIImage>();
    canvas->AddWidget(backgroundImage);
    backgroundImage->SetTextureAndSize(Services::GetAssets()->LoadTexture(config["camBackSprite"].value));
    
    // Position directly below main section.
    backgroundImage->GetRectTransform()->SetAnchor(0.0f, 0.0f);
    backgroundImage->GetRectTransform()->SetPivot(0.0f, 1.0f);
    backgroundImage->GetRectTransform()->SetAnchoredPosition(Vector2::Zero);
    backgroundImage->SetReceivesInput(true);
}

void OptionBar::CreateOptionsSection(UICanvas* canvas, std::unordered_map<std::string, IniKeyValue>& config)
{
    // Make this a child of the "main section" root.
    mOptionsSection = new Actor(Actor::TransformType::RectTransform);
    mOptionsSection->GetTransform()->SetParent(mOptionBarRoot);
    
    // Add background image.
    UIImage* backgroundImage = mOptionsSection->AddComponent<UIImage>();
    canvas->AddWidget(backgroundImage);
    backgroundImage->SetTextureAndSize(Services::GetAssets()->LoadTexture(config["optBackSprite"].value));
    
    // Position directly below main section.
    backgroundImage->GetRectTransform()->SetAnchor(0.0f, 0.0f);
    backgroundImage->GetRectTransform()->SetPivot(0.0f, 1.0f);
    backgroundImage->GetRectTransform()->SetAnchoredPosition(Vector2::Zero);
    backgroundImage->SetReceivesInput(true);
    
    // Create volume slider.
    mGlobalVolumeSlider = CreateSlider(canvas, config, "optSlider", mOptionsSection);
    mGlobalVolumeSlider->SetValueChangeCallback(std::bind(&OptionBar::OnGlobalVolumeSliderValueChanged, this, std::placeholders::_1));
    
    // Create save button.
    UIButton* saveButton = CreateButton(canvas, config, "optSave", mOptionsSection);
    saveButton->SetPressCallback([]() {
        std::cout << "Save!" << std::endl;
    });
    
    // Create restore button.
    UIButton* restoreButton = CreateButton(canvas, config, "optRestore", mOptionsSection);
    restoreButton->SetPressCallback([]() {
        std::cout << "Restore!" << std::endl;
    });
    
    // Created "advanced options" button.
    UIButton* advOptButton = CreateButton(canvas, config, "optAdvanced", mOptionsSection);
    advOptButton->SetPressCallback([this]() {
        this->mAdvancedOptionsSection->SetActive(!this->mAdvancedOptionsSection->IsActive());
        this->KeepOnScreen();
    });
    
    // Create "quit game" button.
    UIButton* quitButton = CreateButton(canvas, config, "optQuit", mOptionsSection);
    quitButton->SetPressCallback([this]() {
        this->Hide();
        //TODO: Show "are you sure you want to quit?" dialog.
        GEngine::Instance()->Quit();
    });
    
    // Create advanced options section.
    CreateAdvancedOptionsSection(canvas, config);
}

void OptionBar::CreateAdvancedOptionsSection(UICanvas* canvas, std::unordered_map<std::string, IniKeyValue>& config)
{
    // Make this a child of the "options section" root.
    mAdvancedOptionsSection = new Actor(Actor::TransformType::RectTransform);
    mAdvancedOptionsSection->GetTransform()->SetParent(mOptionsSection->GetTransform());
    
    // Add background image.
    UIImage* backgroundImage = mAdvancedOptionsSection->AddComponent<UIImage>();
    canvas->AddWidget(backgroundImage);
    backgroundImage->SetTextureAndSize(Services::GetAssets()->LoadTexture(config["advOptBackSprite"].value));
    
    // Position directly below main section.
    backgroundImage->GetRectTransform()->SetAnchor(0.0f, 0.0f);
    backgroundImage->GetRectTransform()->SetPivot(0.0f, 1.0f);
    backgroundImage->GetRectTransform()->SetAnchoredPosition(Vector2::Zero);
    backgroundImage->SetReceivesInput(true);
    
    // Create sound options button.
    UIButton* soundOptsButton = CreateButton(canvas, config, "advOptSound", mAdvancedOptionsSection);
    soundOptsButton->SetPressCallback(std::bind(&OptionBar::ToggleSoundOptions, this));
    
    // Create graphics options button.
    UIButton* graphicsOptsButton = CreateButton(canvas, config, "advOptGraphics", mAdvancedOptionsSection);
    graphicsOptsButton->SetPressCallback([this]() {
        this->mSoundOptionsSection->SetActive(false);
        this->mGraphicOptionsSection->SetActive(!this->mGraphicOptionsSection->IsActive());
        this->mGameOptionsSection->SetActive(false);
        this->KeepOnScreen();
    });
    
    // Create game options button.
    UIButton* gameOptsButton = CreateButton(canvas, config, "advOptGame", mAdvancedOptionsSection);
    gameOptsButton->SetPressCallback([this]() {
        this->mSoundOptionsSection->SetActive(false);
        this->mGraphicOptionsSection->SetActive(false);
        this->mGameOptionsSection->SetActive(!this->mGameOptionsSection->IsActive());
        this->KeepOnScreen();
    });
    
    // Create each subsection...
    CreateSoundOptionsSection(canvas, config);
    CreateGraphicOptionsSection(canvas, config);
    CreateGameOptionsSection(canvas, config);
}

void OptionBar::CreateSoundOptionsSection(UICanvas* canvas, std::unordered_map<std::string, IniKeyValue>& config)
{
    // Make this a child of the "options section" root.
    mSoundOptionsSection = new Actor(Actor::TransformType::RectTransform);
    mSoundOptionsSection->GetTransform()->SetParent(mAdvancedOptionsSection->GetTransform());
    
    // Add background image.
    UIImage* backgroundImage = mSoundOptionsSection->AddComponent<UIImage>();
    canvas->AddWidget(backgroundImage);
    backgroundImage->SetTextureAndSize(Services::GetAssets()->LoadTexture(config["soundOptBackSprite"].value));
    
    // Position directly below main section.
    backgroundImage->GetRectTransform()->SetAnchor(0.0f, 0.0f);
    backgroundImage->GetRectTransform()->SetPivot(0.0f, 1.0f);
    backgroundImage->GetRectTransform()->SetAnchoredPosition(Vector2::Zero);
    backgroundImage->SetReceivesInput(true);

    // Create global volume slider.
    mGlobalVolumeSliderDupe = CreateSlider(canvas, config, "optSoundGlobalSlider", mSoundOptionsSection);
    mGlobalVolumeSliderDupe->SetValueChangeCallback(std::bind(&OptionBar::OnGlobalVolumeSliderValueChanged, this, std::placeholders::_1));

    // Create global mute toggle.
    UIToggle* muteToggle = CreateToggle(canvas, config, "soundOptGlobal", mSoundOptionsSection);
    muteToggle->SetToggleCallback([](bool isOn) {
        Services::GetAudio()->SetMuted(isOn);
    });
    mMuteToggle = muteToggle;
    
    // Create sfx volume slider.
    UISlider* sfxVolumeSlider = CreateSlider(canvas, config, "optSoundSfxSlider", mSoundOptionsSection);
    sfxVolumeSlider->SetValueChangeCallback([](float value) {
        // The SFX volume slider also controls ambient volume.
        Services::GetAudio()->SetVolume(AudioType::SFX, value);
        Services::GetAudio()->SetVolume(AudioType::Ambient, value);
    });

    //Create sfx mute toggle.
    UIToggle* sfxMuteToggle = CreateToggle(canvas, config, "soundOptSfx", mSoundOptionsSection);
    sfxMuteToggle->SetToggleCallback([](bool isOn) {
        // The SFX toggle also controls ambient.
        Services::GetAudio()->SetMuted(AudioType::SFX, isOn);
        Services::GetAudio()->SetMuted(AudioType::Ambient, isOn);
    });
    mSfxMuteToggle = sfxMuteToggle;
    
    // Create VO volume slider.
    UISlider* voVolumeSlider = CreateSlider(canvas, config, "optSoundDialogueSlider", mSoundOptionsSection);
    voVolumeSlider->SetValueChangeCallback([](float value) {
        Services::GetAudio()->SetVolume(AudioType::VO, value);
    });

    // Create VO mute toggle.
    UIToggle* voMuteToggle = CreateToggle(canvas, config, "soundOptDialogue", mSoundOptionsSection);
    voMuteToggle->SetToggleCallback([](bool isOn) {
        Services::GetAudio()->SetMuted(AudioType::VO, isOn);
    });
    mVoMuteToggle = voMuteToggle;
    
    // Create music volume slider.
    UISlider* musicVolumeSlider = CreateSlider(canvas, config, "optSoundMusicSlider", mSoundOptionsSection);
    musicVolumeSlider->SetValueChangeCallback([](float value) {
        Services::GetAudio()->SetVolume(AudioType::Music, value);
    });

    // Create music mute toggle.
    UIToggle* musicMuteToggle = CreateToggle(canvas, config, "soundOptMusic", mSoundOptionsSection);
    musicMuteToggle->SetToggleCallback([](bool isOn) {
        Services::GetAudio()->SetMuted(AudioType::Music, isOn);
    });
    mMusicMuteToggle = musicMuteToggle;
    
    // Create max sounds slider.
    UISlider* maxSoundsSlider = CreateSlider(canvas, config, "optSoundMusicSlider", mSoundOptionsSection);
    maxSoundsSlider->SetValueChangeCallback([](float value) {
        std::cout << "Changed max sounds to " << value << std::endl;
    });
}

void OptionBar::CreateGraphicOptionsSection(UICanvas* canvas, std::unordered_map<std::string, IniKeyValue>& config)
{
    // Make this a child of the "options section" root.
    mGraphicOptionsSection = new Actor(Actor::TransformType::RectTransform);
    mGraphicOptionsSection->GetTransform()->SetParent(mAdvancedOptionsSection->GetTransform());
    
    // Add background image.
    UIImage* backgroundImage = mGraphicOptionsSection->AddComponent<UIImage>();
    canvas->AddWidget(backgroundImage);
    backgroundImage->SetTextureAndSize(Services::GetAssets()->LoadTexture(config["graphicsOptBackSprite"].value));
    
    // Position directly below main section.
    backgroundImage->GetRectTransform()->SetAnchor(0.0f, 0.0f);
    backgroundImage->GetRectTransform()->SetPivot(0.0f, 1.0f);
    backgroundImage->GetRectTransform()->SetAnchoredPosition(Vector2::Zero);
    backgroundImage->SetReceivesInput(true);
    
    //TODO: Gamma slider
    
    //TODO: 3D driver dropdown
    
    //TODO: Resolution dropdown
    
    // Create "incremental rendering" text (text can be "disabled", like a button, if graphics system doesn't support this option).
    UIButton* incRenderingText = CreateButton(canvas, config, "graphOptIncrementalText", mGraphicOptionsSection, false);
    incRenderingText->SetUpTexture(Services::GetAssets()->LoadTexture(config["graphOptIncrementalEnabled"].value));
    incRenderingText->SetDisabledTexture(Services::GetAssets()->LoadTexture(config["graphOptIncrementalDisabled"].value));
    incRenderingText->SetReceivesInput(false);
    
    // Create "incremental rendering" toggle.
    UIToggle* incRenderingToggle = CreateToggle(canvas, config, "graphOptIncremental", mGraphicOptionsSection);
    incRenderingToggle->SetToggleCallback([](bool isOn) {
        std::cout << "Incremental rendering is toggled " << isOn << std::endl;
    });
    
    // Create advanced graphics options button.
    UIButton* advGraphicsButton = CreateButton(canvas, config, "graphOptAdvanced", mGraphicOptionsSection);
    advGraphicsButton->SetPressCallback([this]() {
        this->mAdvancedGraphicOptionsSection->SetActive(!this->mAdvancedGraphicOptionsSection->IsActive());
        this->KeepOnScreen();
    });
    
    CreateAdvancedGraphicOptionsSection(canvas, config);
}

void OptionBar::CreateAdvancedGraphicOptionsSection(UICanvas* canvas, std::unordered_map<std::string, IniKeyValue>& config)
{
    // Make this a child of the "options section" root.
    mAdvancedGraphicOptionsSection = new Actor(Actor::TransformType::RectTransform);
    mAdvancedGraphicOptionsSection->GetTransform()->SetParent(mGraphicOptionsSection->GetTransform());
    
    // Add background image.
    UIImage* backgroundImage = mAdvancedGraphicOptionsSection->AddComponent<UIImage>();
    canvas->AddWidget(backgroundImage);
    backgroundImage->SetTextureAndSize(Services::GetAssets()->LoadTexture(config["advGraphOptBackSprite"].value));
    
    // Position directly below main section.
    backgroundImage->GetRectTransform()->SetAnchor(0.0f, 0.0f);
    backgroundImage->GetRectTransform()->SetPivot(0.0f, 1.0f);
    backgroundImage->GetRectTransform()->SetAnchoredPosition(Vector2::Zero);
    backgroundImage->SetReceivesInput(true);
    
    //TODO: Texture quality dropdown
    //TODO: Lighting quality dropdown
    
    // Create "mipmap" text (text can be "disabled", like a button, if graphics system doesn't support this option).
    UIButton* mipmapText = CreateButton(canvas, config, "advGraphOptMipMapText", mAdvancedGraphicOptionsSection, false);
    mipmapText->SetUpTexture(Services::GetAssets()->LoadTexture(config["advGraphOptMipMapEnabled"].value));
    mipmapText->SetDisabledTexture(Services::GetAssets()->LoadTexture(config["advGraphOptMipMapDisabled"].value));
    mipmapText->SetReceivesInput(false);
    
    // Create "mipmap" toggle.
    UIToggle* mipmapToggle = CreateToggle(canvas, config, "advGraphOptMipMap", mAdvancedGraphicOptionsSection);
    mipmapToggle->SetToggleCallback([](bool isOn) {
        std::cout << "Mipmap is toggled " << isOn << std::endl;
    });
    
    // Create "animation interpolation" text (text can be "disabled", like a button, if graphics system doesn't support this option).
    UIButton* animInterpText = CreateButton(canvas, config, "advGraphOptInterpolationText", mAdvancedGraphicOptionsSection, false);
    animInterpText->SetUpTexture(Services::GetAssets()->LoadTexture(config["advGraphOptInterpolationEnabled"].value));
    animInterpText->SetDisabledTexture(Services::GetAssets()->LoadTexture(config["advGraphOptInterpolationDisabled"].value));
    animInterpText->SetReceivesInput(false);
    
    // Create "animation interpolation" toggle.
    UIToggle* animInterpToggle = CreateToggle(canvas, config, "advGraphOptInterpolation", mAdvancedGraphicOptionsSection);
    animInterpToggle->SetToggleCallback([](bool isOn) {
        std::cout << "Anim interp is toggled " << isOn << std::endl;
    });
    
    // Create "trilinear filtering" text (text can be "disabled", like a button, if graphics system doesn't support this option).
    UIButton* filteringText = CreateButton(canvas, config, "advGraphOptFilteringText", mAdvancedGraphicOptionsSection, false);
    filteringText->SetUpTexture(Services::GetAssets()->LoadTexture(config["advGraphOptFilteringEnabled"].value));
    filteringText->SetDisabledTexture(Services::GetAssets()->LoadTexture(config["advGraphOptFilteringDisabled"].value));
    filteringText->SetReceivesInput(false);
    
    // Create "trilinear filtering" toggle.
    UIToggle* filteringToggle = CreateToggle(canvas, config, "advGraphOptFiltering", mAdvancedGraphicOptionsSection);
    filteringToggle->SetToggleCallback([](bool isOn) {
        std::cout << "Trilinear filtering is toggled " << isOn << std::endl;
    });
    
    // Create "lod" text (text can be "disabled", like a button, if graphics system doesn't support this option).
    UIButton* lodText = CreateButton(canvas, config, "advGraphOptLODText", mAdvancedGraphicOptionsSection, false);
    lodText->SetUpTexture(Services::GetAssets()->LoadTexture(config["advGraphOptLODEnabled"].value));
    lodText->SetDisabledTexture(Services::GetAssets()->LoadTexture(config["advGraphOptLODDisabled"].value));
    lodText->SetReceivesInput(false);
    
    //TODO: LOD Slider
}

void OptionBar::CreateGameOptionsSection(UICanvas* canvas, std::unordered_map<std::string, IniKeyValue>& config)
{
    // Make this a child of the "options section" root.
    mGameOptionsSection = new Actor(Actor::TransformType::RectTransform);
    mGameOptionsSection->GetTransform()->SetParent(mAdvancedOptionsSection->GetTransform());
    
    // Add background image.
    UIImage* backgroundImage = mGameOptionsSection->AddComponent<UIImage>();
    canvas->AddWidget(backgroundImage);
    backgroundImage->SetTextureAndSize(Services::GetAssets()->LoadTexture(config["gameOptBackSprite"].value));
    
    // Position directly below main section.
    backgroundImage->GetRectTransform()->SetAnchor(0.0f, 0.0f);
    backgroundImage->GetRectTransform()->SetPivot(0.0f, 1.0f);
    backgroundImage->GetRectTransform()->SetAnchoredPosition(Vector2::Zero);
    backgroundImage->SetReceivesInput(true);
    
    // Create glide camera toggle.
    UIToggle* glideCameraToggle = CreateToggle(canvas, config, "gameOptGlide", mGameOptionsSection);
    
    // Create captions toggle.
    UIToggle* captionsToggle = CreateToggle(canvas, config, "gameOptCaptions", mGameOptionsSection);
    
    // Create keyboard controls button.
    UIButton* controlsButton = CreateButton(canvas, config, "gameOptControls", mGameOptionsSection);
    controlsButton->SetPressCallback([this]() {
        std::cout << "Controls" << std::endl;
    });
}

void OptionBar::ToggleSoundOptions()
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
        mMuteToggle->SetValue(Services::GetAudio()->GetMuted());
        mSfxMuteToggle->SetValue(Services::GetAudio()->GetMuted(AudioType::SFX));
        mVoMuteToggle->SetValue(Services::GetAudio()->GetMuted(AudioType::VO));
        mMusicMuteToggle->SetValue(Services::GetAudio()->GetMuted(AudioType::Music));
    }
}

void OptionBar::OnGlobalVolumeSliderValueChanged(float value)
{
    // Change the volume.
    Services::GetAudio()->SetMasterVolume(value);

    // Since there are two (!!) sliders for global volume, make sure they stay in-sync.
    mGlobalVolumeSlider->SetValueSilently(value);
    mGlobalVolumeSliderDupe->SetValueSilently(value);
}