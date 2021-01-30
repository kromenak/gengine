//
// OptionBar.cpp
//
// Clark Kromenaker
//
#include "OptionBar.h"

#include "InventoryManager.h"
#include "Services.h"
#include "UIButton.h"
#include "UICanvas.h"
#include "UIImage.h"
#include "UILabel.h"
#include "UIToggle.h"

OptionBar::OptionBar() : Actor(Actor::TransformType::RectTransform)
{
    // Load layout text file, parse to key/value map, and then delete it.
    TextAsset* optionBarText = Services::GetAssets()->LoadText("RC_LAYOUT.TXT");
    
    IniParser parser(optionBarText->GetText(), optionBarText->GetTextLength());
    parser.SetMultipleKeyValuePairsPerLine(false);
    std::unordered_map<std::string, IniKeyValue> optionBarConfig = parser.ParseAllAsMap();
    
    delete optionBarText;
    
    // Create canvas, to contain the UI components.
    UICanvas* canvas = AddComponent<UICanvas>();
    
    // Create sections.
    CreateMainSection(canvas, optionBarConfig);
    CreateCamerasSection(canvas, optionBarConfig);
    CreateOptionsSection(canvas, optionBarConfig);
    
    // Hide by default.
    Hide();
}

void OptionBar::Show()
{
    SetActive(true);
    
    // Position option bar over mouse.
    // "Minus half size" because option bar's pivot is lower-left corner, but want mouse at center.
    // Also, round the "half size" to ensure the UI renders "pixel perfect" - on exact pixel spots rather than between them.
    Vector2 halfSize = mRootRectTransform->GetSize() * 0.5f;
    halfSize.x = Math::Round(halfSize.x);
    halfSize.y = Math::Round(halfSize.y);
    mRootRectTransform->SetAnchoredPosition(Services::GetInput()->GetMousePosition() - halfSize);
    
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
	if(Services::GetInput()->IsKeyDown(SDL_SCANCODE_L))
    {
        RectTransform* rt = static_cast<RectTransform*>(GetTransform());
        Vector2 ap = rt->GetAnchoredPosition();
        ap.x += 10;
        ap.y += 10;
        rt->SetAnchoredPosition(ap);
    }
    if(Services::GetInput()->IsKeyDown(SDL_SCANCODE_K))
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
    mRootRectTransform->MoveInsideRect(Services::GetRenderer()->GetScreenRect());
}

UIButton* CreateButton(std::unordered_map<std::string, IniKeyValue>& config, const std::string& buttonId, bool setSprites = true)
{
    Actor* buttonActor = new Actor(Actor::TransformType::RectTransform);
    UIButton* button = buttonActor->AddComponent<UIButton>();
    button->GetRectTransform()->SetAnchor(0.0f, 1.0f);
    button->GetRectTransform()->SetPivot(0.0f, 1.0f);
    
    Vector2 buttonPos = config[buttonId + "Pos"].GetValueAsVector2();
    buttonPos.y *= -1;
    button->GetRectTransform()->SetAnchoredPosition(buttonPos);
    
    if(setSprites)
    {
        button->SetUpTexture(Services::GetAssets()->LoadTexture(config[buttonId + "SpriteUp"].value));
        button->SetDownTexture(Services::GetAssets()->LoadTexture(config[buttonId + "SpriteDown"].value));
        button->SetDisabledTexture(Services::GetAssets()->LoadTexture(config[buttonId + "SpriteDis"].value));
        button->SetHoverTexture(Services::GetAssets()->LoadTexture(config[buttonId + "SpriteHov"].value));
    }
    return button;
}

UIToggle* CreateToggle(std::unordered_map<std::string, IniKeyValue>& config, const std::string& toggleId)
{
    Actor* toggleActor = new Actor(Actor::TransformType::RectTransform);
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

void OptionBar::CreateMainSection(UICanvas* canvas, std::unordered_map<std::string, IniKeyValue>& config)
{
    // Add background image.
    UIImage* backgroundImage = AddComponent<UIImage>();
    canvas->AddWidget(backgroundImage);
    backgroundImage->SetTextureAndSize(Services::GetAssets()->LoadTexture(config["backSprite"].value));
    
    // Some positioning code would be easier if this was (0.5, 0.5) BUT that causes some rendering problems b/c the rect is no longer pixel perfect.
    // Mayyybe we can fix that in the RectTransform/Rect code directly...with a bit of work.
    backgroundImage->GetRectTransform()->SetPivot(Vector2::Zero);
    
    backgroundImage->GetRectTransform()->SetAnchor(Vector2::Zero);
    backgroundImage->GetRectTransform()->SetAnchoredPosition(Vector2::Zero);
    
    backgroundImage->SetReceivesInput(true);
    
    // Cache root rect transform for use later on.
    mRootRectTransform = backgroundImage->GetRectTransform();
    
    // Load font.
    Font* font = Services::GetAssets()->LoadFont(config["statusFont"].value);
    
    // Add score text.
    Actor* scoreActor = new Actor(Actor::TransformType::RectTransform);
    scoreActor->GetTransform()->SetParent(GetTransform());
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
    dayActor->GetTransform()->SetParent(GetTransform());
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
    timeActor->GetTransform()->SetParent(GetTransform());
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
    UIButton* invButton = CreateButton(config, "closed");
    invButton->GetRectTransform()->SetParent(GetTransform());
    canvas->AddWidget(invButton);
    
    invButton->SetPressCallback([this]() {
        this->Hide();
        Services::Get<InventoryManager>()->ShowInventory();
    });
    
    // Add hint button.
    UIButton* hintButton = CreateButton(config, "hint");
    hintButton->GetRectTransform()->SetParent(GetTransform());
    canvas->AddWidget(hintButton);
    
    hintButton->SetPressCallback([]() {
        std::cout << "Hint!" << std::endl;
    });
    
    // Add camera button.
    UIButton* cameraButton = CreateButton(config, "camera");
    cameraButton->GetRectTransform()->SetParent(GetTransform());
    canvas->AddWidget(cameraButton);
    
    cameraButton->SetPressCallback([this]() {
        this->mCamerasSection->SetActive(!this->mCamerasSection->IsActive());
        this->mOptionsSection->SetActive(false);
        this->KeepOnScreen();
    });
    
    // Add cinematics button.
    UIButton* cinematicsOnButton = CreateButton(config, "cine");
    cinematicsOnButton->GetRectTransform()->SetParent(GetTransform());
    canvas->AddWidget(cinematicsOnButton);
    
    cinematicsOnButton->SetPressCallback([]() {
        std::cout << "Turn cinematics on!" << std::endl;
    });
    
    UIButton* cinematicsOffButton = CreateButton(config, "cineoff");
    cinematicsOffButton->GetRectTransform()->SetParent(GetTransform());
    canvas->AddWidget(cinematicsOffButton);
    
    cinematicsOffButton->SetPressCallback([]() {
        std::cout << "Turn cinematics off!" << std::endl;
    });
    
    // Add help button.
    UIButton* helpButton = CreateButton(config, "help");
    helpButton->GetRectTransform()->SetParent(GetTransform());
    canvas->AddWidget(helpButton);
    
    helpButton->SetPressCallback([]() {
        std::cout << "Help!" << std::endl;
    });
    
    // Add options button.
    UIButton* optionsButton = CreateButton(config, "options");
    optionsButton->GetRectTransform()->SetParent(GetTransform());
    canvas->AddWidget(optionsButton);
    
    optionsButton->SetPressCallback([this]() {
        this->mCamerasSection->SetActive(false);
        this->mOptionsSection->SetActive(!this->mOptionsSection->IsActive());
        this->KeepOnScreen();
    });
    
    // Add close button.
    UIButton* closeButton = CreateButton(config, "exit");
    closeButton->GetRectTransform()->SetParent(GetTransform());
    canvas->AddWidget(closeButton);
    
    closeButton->SetPressCallback([this]() {
        this->Hide();
    });
}

void OptionBar::CreateCamerasSection(UICanvas* canvas, std::unordered_map<std::string, IniKeyValue>& config)
{
    // Make this a child of the "main section" root.
    mCamerasSection = new Actor(Actor::TransformType::RectTransform);
    mCamerasSection->GetTransform()->SetParent(canvas->GetRectTransform());
    
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
    mOptionsSection->GetTransform()->SetParent(canvas->GetRectTransform());
    
    // Add background image.
    UIImage* backgroundImage = mOptionsSection->AddComponent<UIImage>();
    canvas->AddWidget(backgroundImage);
    backgroundImage->SetTextureAndSize(Services::GetAssets()->LoadTexture(config["optBackSprite"].value));
    
    // Position directly below main section.
    backgroundImage->GetRectTransform()->SetAnchor(0.0f, 0.0f);
    backgroundImage->GetRectTransform()->SetPivot(0.0f, 1.0f);
    backgroundImage->GetRectTransform()->SetAnchoredPosition(Vector2::Zero);
    backgroundImage->SetReceivesInput(true);
    
    //TODO: Create volume slider
    
    // Create save button.
    UIButton* saveButton = CreateButton(config, "optSave");
    saveButton->GetRectTransform()->SetParent(mOptionsSection->GetTransform());
    canvas->AddWidget(saveButton);
    
    saveButton->SetPressCallback([]() {
        std::cout << "Save!" << std::endl;
    });
    
    // Create restore button.
    UIButton* restoreButton = CreateButton(config, "optRestore");
    restoreButton->GetRectTransform()->SetParent(mOptionsSection->GetTransform());
    canvas->AddWidget(restoreButton);
    
    restoreButton->SetPressCallback([]() {
        std::cout << "Restore!" << std::endl;
    });
    
    // Created "advanced options" button.
    UIButton* advOptButton = CreateButton(config, "optAdvanced");
    advOptButton->GetRectTransform()->SetParent(mOptionsSection->GetTransform());
    canvas->AddWidget(advOptButton);
    
    advOptButton->SetPressCallback([this]() {
        this->mAdvancedOptionsSection->SetActive(!this->mAdvancedOptionsSection->IsActive());
        this->KeepOnScreen();
    });
    
    // Create "quit game" button.
    UIButton* quitButton = CreateButton(config, "optQuit");
    quitButton->GetRectTransform()->SetParent(mOptionsSection->GetTransform());
    canvas->AddWidget(quitButton);
    
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
    UIButton* soundOptsButton = CreateButton(config, "advOptSound");
    soundOptsButton->GetRectTransform()->SetParent(mAdvancedOptionsSection->GetTransform());
    canvas->AddWidget(soundOptsButton);
    
    soundOptsButton->SetPressCallback([this]() {
        this->mSoundOptionsSection->SetActive(!this->mSoundOptionsSection->IsActive());
        this->mGraphicOptionsSection->SetActive(false);
        this->mGameOptionsSection->SetActive(false);
        this->KeepOnScreen();
    });
    
    // Create graphics options button.
    UIButton* graphicsOptsButton = CreateButton(config, "advOptGraphics");
    graphicsOptsButton->GetRectTransform()->SetParent(mAdvancedOptionsSection->GetTransform());
    canvas->AddWidget(graphicsOptsButton);
    
    graphicsOptsButton->SetPressCallback([this]() {
        this->mSoundOptionsSection->SetActive(false);
        this->mGraphicOptionsSection->SetActive(!this->mGraphicOptionsSection->IsActive());
        this->mGameOptionsSection->SetActive(false);
        this->KeepOnScreen();
    });
    
    // Create game options button.
    UIButton* gameOptsButton = CreateButton(config, "advOptGame");
    gameOptsButton->GetRectTransform()->SetParent(mAdvancedOptionsSection->GetTransform());
    canvas->AddWidget(gameOptsButton);
    
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
    
    //TODO: Global volume slider
    //Create global mute toggle.
    UIToggle* muteToggle = CreateToggle(config, "soundOptGlobal");
    muteToggle->GetRectTransform()->SetParent(mSoundOptionsSection->GetTransform());
    canvas->AddWidget(muteToggle);
    
    muteToggle->SetToggleCallback([](bool isOn) {
        std::cout << "Mute global toggled " << isOn << std::endl;
    });
    
    //TODO: SFX volume slider
    //Create sfx mute toggle.
    UIToggle* sfxMuteToggle = CreateToggle(config, "soundOptSfx");
    sfxMuteToggle->GetRectTransform()->SetParent(mSoundOptionsSection->GetTransform());
    canvas->AddWidget(sfxMuteToggle);
    
    sfxMuteToggle->SetToggleCallback([](bool isOn) {
        std::cout << "Mute SFX toggled " << isOn << std::endl;
    });
    
    //TODO: VO volume slider
    //Create VO mute toggle.
    UIToggle* voMuteToggle = CreateToggle(config, "soundOptDialogue");
    voMuteToggle->GetRectTransform()->SetParent(mSoundOptionsSection->GetTransform());
    canvas->AddWidget(voMuteToggle);
    
    voMuteToggle->SetToggleCallback([](bool isOn) {
        std::cout << "Mute VO toggled " << isOn << std::endl;
    });
    
    //TODO: Music volume slider
    //Create music mute toggle.
    UIToggle* musicMuteToggle = CreateToggle(config, "soundOptMusic");
    musicMuteToggle->GetRectTransform()->SetParent(mSoundOptionsSection->GetTransform());
    canvas->AddWidget(musicMuteToggle);
    
    musicMuteToggle->SetToggleCallback([](bool isOn) {
        std::cout << "Mute music toggled " << isOn << std::endl;
    });
    
    //TODO: Max sounds slider
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
    UIButton* incRenderingText = CreateButton(config, "graphOptIncrementalText", false);
    incRenderingText->GetRectTransform()->SetParent(mGraphicOptionsSection->GetTransform());
    canvas->AddWidget(incRenderingText);
    incRenderingText->SetUpTexture(Services::GetAssets()->LoadTexture(config["graphOptIncrementalEnabled"].value));
    incRenderingText->SetDisabledTexture(Services::GetAssets()->LoadTexture(config["graphOptIncrementalDisabled"].value));
    incRenderingText->SetReceivesInput(false);
    
    // Create "incremental rendering" toggle.
    UIToggle* incRenderingToggle = CreateToggle(config, "graphOptIncremental");
    incRenderingToggle->GetRectTransform()->SetParent(mGraphicOptionsSection->GetTransform());
    canvas->AddWidget(incRenderingToggle);
    
    incRenderingToggle->SetToggleCallback([](bool isOn) {
        std::cout << "Incremental rendering is toggled " << isOn << std::endl;
    });
    
    // Create advanced graphics options button.
    UIButton* advGraphicsButton = CreateButton(config, "graphOptAdvanced");
    advGraphicsButton->GetRectTransform()->SetParent(mGraphicOptionsSection->GetTransform());
    canvas->AddWidget(advGraphicsButton);
    
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
    UIButton* mipmapText = CreateButton(config, "advGraphOptMipMapText", false);
    mipmapText->GetRectTransform()->SetParent(mAdvancedGraphicOptionsSection->GetTransform());
    canvas->AddWidget(mipmapText);
    mipmapText->SetUpTexture(Services::GetAssets()->LoadTexture(config["advGraphOptMipMapEnabled"].value));
    mipmapText->SetDisabledTexture(Services::GetAssets()->LoadTexture(config["advGraphOptMipMapDisabled"].value));
    mipmapText->SetReceivesInput(false);
    
    // Create "mipmap" toggle.
    UIToggle* mipmapToggle = CreateToggle(config, "advGraphOptMipMap");
    mipmapToggle->GetRectTransform()->SetParent(mAdvancedGraphicOptionsSection->GetTransform());
    canvas->AddWidget(mipmapToggle);
    
    mipmapToggle->SetToggleCallback([](bool isOn) {
        std::cout << "Mipmap is toggled " << isOn << std::endl;
    });
    
    // Create "animation interpolation" text (text can be "disabled", like a button, if graphics system doesn't support this option).
    UIButton* animInterpText = CreateButton(config, "advGraphOptInterpolationText", false);
    animInterpText->GetRectTransform()->SetParent(mAdvancedGraphicOptionsSection->GetTransform());
    canvas->AddWidget(animInterpText);
    animInterpText->SetUpTexture(Services::GetAssets()->LoadTexture(config["advGraphOptInterpolationEnabled"].value));
    animInterpText->SetDisabledTexture(Services::GetAssets()->LoadTexture(config["advGraphOptInterpolationDisabled"].value));
    animInterpText->SetReceivesInput(false);
    
    // Create "animation interpolation" toggle.
    UIToggle* animInterpToggle = CreateToggle(config, "advGraphOptInterpolation");
    animInterpToggle->GetRectTransform()->SetParent(mAdvancedGraphicOptionsSection->GetTransform());
    canvas->AddWidget(animInterpToggle);
    
    animInterpToggle->SetToggleCallback([](bool isOn) {
        std::cout << "Anim interp is toggled " << isOn << std::endl;
    });
    
    // Create "trilinear filtering" text (text can be "disabled", like a button, if graphics system doesn't support this option).
    UIButton* filteringText = CreateButton(config, "advGraphOptFilteringText", false);
    filteringText->GetRectTransform()->SetParent(mAdvancedGraphicOptionsSection->GetTransform());
    canvas->AddWidget(filteringText);
    filteringText->SetUpTexture(Services::GetAssets()->LoadTexture(config["advGraphOptFilteringEnabled"].value));
    filteringText->SetDisabledTexture(Services::GetAssets()->LoadTexture(config["advGraphOptFilteringDisabled"].value));
    filteringText->SetReceivesInput(false);
    
    // Create "trilinear filtering" toggle.
    UIToggle* filteringToggle = CreateToggle(config, "advGraphOptFiltering");
    filteringToggle->GetRectTransform()->SetParent(mAdvancedGraphicOptionsSection->GetTransform());
    canvas->AddWidget(filteringToggle);
    
    filteringToggle->SetToggleCallback([](bool isOn) {
        std::cout << "Trilinear filtering is toggled " << isOn << std::endl;
    });
    
    // Create "lod" text (text can be "disabled", like a button, if graphics system doesn't support this option).
    UIButton* lodText = CreateButton(config, "advGraphOptLODText", false);
    lodText->GetRectTransform()->SetParent(mAdvancedGraphicOptionsSection->GetTransform());
    canvas->AddWidget(lodText);
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
    UIToggle* glideCameraToggle = CreateToggle(config, "gameOptGlide");
    glideCameraToggle->GetRectTransform()->SetParent(mGameOptionsSection->GetTransform());
    canvas->AddWidget(glideCameraToggle);
    
    // Create captions toggle.
    UIToggle* captionsToggle = CreateToggle(config, "gameOptCaptions");
    captionsToggle->GetRectTransform()->SetParent(mGameOptionsSection->GetTransform());
    canvas->AddWidget(captionsToggle);
    
    // Create keyboard controls button.
    UIButton* controlsButton = CreateButton(config, "gameOptControls");
    controlsButton->GetRectTransform()->SetParent(mGameOptionsSection->GetTransform());
    canvas->AddWidget(controlsButton);
    
    controlsButton->SetPressCallback([this]() {
        std::cout << "Controls" << std::endl;
    });
}
