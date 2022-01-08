#include "TitleScreen.h"

#include "Services.h"
#include "UIButton.h"
#include "UICanvas.h"
#include "UIImage.h"
#include "VideoPlayer.h"

UIButton* CreateButton(UICanvas* canvas, const std::string& buttonId, float xPos)
{
    Actor* buttonActor = new Actor(Actor::TransformType::RectTransform);
    UIButton* button = buttonActor->AddComponent<UIButton>();
    canvas->AddWidget(button);

    // Set textures.
    button->SetUpTexture(Services::GetAssets()->LoadTexture(buttonId + "_U.BMP"));
    button->SetDownTexture(Services::GetAssets()->LoadTexture(buttonId + "_D.BMP"));
    button->SetHoverTexture(Services::GetAssets()->LoadTexture(buttonId + "_H.BMP"));
    button->SetDisabledTexture(Services::GetAssets()->LoadTexture(buttonId + "_X.BMP"));

    // Anchor to bottom-right and position based off that.
    button->GetRectTransform()->SetAnchor(1.0f, 0.0f);
    button->GetRectTransform()->SetPivot(0.0f, 1.0f);
    button->GetRectTransform()->SetAnchoredPosition(xPos, 67.0f);
    return button;
}

TitleScreen::TitleScreen() : Actor(Actor::TransformType::RectTransform)
{
    UICanvas* canvas = AddComponent<UICanvas>(0);

    // Canvas takes up entire screen.
    RectTransform* rectTransform = GetComponent<RectTransform>();
    rectTransform->SetSizeDelta(0.0f, 0.0f);
    rectTransform->SetAnchorMin(Vector2::Zero);
    rectTransform->SetAnchorMax(Vector2::One);

    // Add title screen background image.
    UIImage* background = AddComponent<UIImage>();
    canvas->AddWidget(background);
    background->SetTexture(Services::GetAssets()->LoadTexture("TITLE.BMP"));

    // Add "intro" button.
    UIButton* introButton = CreateButton(canvas, "TITLE_INTRO", -505.0f);
    introButton->SetPressCallback([]() {
        Services::Get<VideoPlayer>()->Play("intro.bik", true, true, nullptr);
    });

    // Add "play" button.
    UIButton* playButton = CreateButton(canvas, "TITLE_PLAY", -381.0f);
    playButton->SetPressCallback([]() {
        GEngine::Instance()->StartGame();
    });

    // Add "restore" button.
    UIButton* restoreButton = CreateButton(canvas, "TITLE_RESTORE", -257.0f);
    restoreButton->SetPressCallback([]() {
        std::cout << "Restore!" << std::endl;
    });

    // Add "quit" button.
    UIButton* quitButton = CreateButton(canvas, "TITLE_QUIT", -135.0f);
    quitButton->SetPressCallback([]() {
        GEngine::Instance()->Quit();
    });
}