#include "Sidney.h"

#include "Texture.h"
#include "UIButton.h"
#include "UICanvas.h"
#include "UIImage.h"
#include "UILabel.h"

UIButton* CreateMainButton(UICanvas* canvas, Actor* parent, const std::string& buttonId, float xPos)
{
    Actor* actor = new Actor(Actor::TransformType::RectTransform);
    actor->GetTransform()->SetParent(parent->GetTransform());
    UIButton* button = actor->AddComponent<UIButton>();
    canvas->AddWidget(button);

    button->GetRectTransform()->SetPivot(0.0f, 1.0f);
    button->GetRectTransform()->SetAnchor(0.0f, 1.0f);
    button->GetRectTransform()->SetAnchoredPosition(xPos, -24.0f);

    button->SetUpTexture(Services::GetAssets()->LoadTexture("B_" + buttonId + "_U.BMP"));
    button->SetHoverTexture(Services::GetAssets()->LoadTexture("B_" + buttonId + "_H.BMP"));
    button->SetDownTexture(Services::GetAssets()->LoadTexture("B_" + buttonId + "_D.BMP"));
    button->SetDisabledTexture(Services::GetAssets()->LoadTexture("B_" + buttonId + "_X.BMP"));
    return button;
}

Sidney::Sidney() : Actor(Actor::TransformType::RectTransform)
{
    // Let's put Sidney at same draw order as Driving Screen.
    mCanvas = AddComponent<UICanvas>(4);

    // Canvas takes up entire screen.
    RectTransform* rectTransform = GetComponent<RectTransform>();
    rectTransform->SetSizeDelta(0.0f, 0.0f);
    rectTransform->SetAnchorMin(Vector2::Zero);
    rectTransform->SetAnchorMax(Vector2::One);

    // Add black background that eats input.
    UIImage* background = AddComponent<UIImage>();
    mCanvas->AddWidget(background);
    background->SetTexture(&Texture::Black);
    background->SetReceivesInput(true);

    // Add desktop background image.
    Actor* desktopBackground = new Actor(Actor::TransformType::RectTransform);
    desktopBackground->GetTransform()->SetParent(GetTransform());
    UIImage* desktopBackgroundImage = desktopBackground->AddComponent<UIImage>();
    mCanvas->AddWidget(desktopBackgroundImage);
    desktopBackgroundImage->SetTexture(Services::GetAssets()->LoadTexture("S_MAIN_SCN.BMP"), true);

    // Add exit button as child of desktop background.
    {
        Actor* exitButtonActor = new Actor(Actor::TransformType::RectTransform);
        exitButtonActor->GetTransform()->SetParent(desktopBackground->GetTransform());
        UIButton* exitButton = exitButtonActor->AddComponent<UIButton>();
        mCanvas->AddWidget(exitButton);

        exitButton->GetRectTransform()->SetPivot(1.0f, 0.0f); // Bottom-Right
        exitButton->GetRectTransform()->SetAnchor(1.0f, 0.0f); // Bottom-Right
        exitButton->GetRectTransform()->SetAnchoredPosition(-10.0f, 10.0f); // 10x10 offset from Bottom-Right
        exitButton->GetRectTransform()->SetSizeDelta(80.0f, 18.0f);

        //TODO: Exit button uses a stretch-based image solution (kind of like 9-slice, more like 3-slice).
        exitButton->SetResizeBasedOnTexture(false);
        exitButton->SetUpTexture(&Texture::White);

        exitButton->SetPressCallback([this](UIButton* button){
            Services::GetAudio()->PlaySFX(Services::GetAssets()->LoadAudio("SIDBUTTON3.WAV"));
            Hide();
        });

        // Add exit button text.
        UILabel* exitLabel = exitButtonActor->AddComponent<UILabel>();
        mCanvas->AddWidget(exitLabel);
        exitLabel->SetFont(Services::GetAssets()->LoadFont("SID_TEXT_18.FON"));
        exitLabel->SetText("EXIT");
        exitLabel->SetHorizonalAlignment(HorizontalAlignment::Center);
        exitLabel->SetVerticalAlignment(VerticalAlignment::Center);
    }

    // Add button bar for subscreens.
    {
        const float kButtonStart = 5.0f;
        const float kButtonSpacing = 79.0f;

        float buttonPos = kButtonStart;
        UIButton* searchButton = CreateMainButton(mCanvas, desktopBackground, "SEARCH", buttonPos);
        searchButton->SetPressCallback([this](UIButton* button){
            Services::GetAudio()->PlaySFX(Services::GetAssets()->LoadAudio("SIDENTER.WAV"));
            mSearch.Show();
        });

        buttonPos += kButtonSpacing;
        UIButton* emailButton = CreateMainButton(mCanvas, desktopBackground, "EMAIL", buttonPos);
        emailButton->SetPressCallback([](UIButton* button){
            Services::GetAudio()->PlaySFX(Services::GetAssets()->LoadAudio("SIDENTER.WAV"));
            printf("Email\n");
        });

        buttonPos += kButtonSpacing;
        UIButton* filesButton = CreateMainButton(mCanvas, desktopBackground, "FILES", buttonPos);
        filesButton->SetPressCallback([](UIButton* button){
            Services::GetAudio()->PlaySFX(Services::GetAssets()->LoadAudio("SIDENTER.WAV"));
            printf("Files\n");
        });

        buttonPos += kButtonSpacing;
        UIButton* analyzeButton = CreateMainButton(mCanvas, desktopBackground, "ANALYZE", buttonPos);
        analyzeButton->SetPressCallback([](UIButton* button){
            Services::GetAudio()->PlaySFX(Services::GetAssets()->LoadAudio("SIDENTER.WAV"));
            printf("Analyze\n");
        });

        buttonPos += kButtonSpacing;
        UIButton* translateButton = CreateMainButton(mCanvas, desktopBackground, "TRANSL", buttonPos);
        translateButton->SetPressCallback([](UIButton* button){
            Services::GetAudio()->PlaySFX(Services::GetAssets()->LoadAudio("SIDENTER.WAV"));
            printf("Translate\n");
        });

        buttonPos += kButtonSpacing;
        UIButton* dataButton = CreateMainButton(mCanvas, desktopBackground, "ADDATA", buttonPos);
        dataButton->SetPressCallback([](UIButton* button){
            Services::GetAudio()->PlaySFX(Services::GetAssets()->LoadAudio("SIDENTER.WAV"));
            printf("Add Data\n");
        });

        buttonPos += kButtonSpacing;
        UIButton* idButton = CreateMainButton(mCanvas, desktopBackground, "MAKEID", buttonPos);
        idButton->SetPressCallback([](UIButton* button){
            Services::GetAudio()->PlaySFX(Services::GetAssets()->LoadAudio("SIDENTER.WAV"));
            printf("Make ID\n");
        });

        buttonPos += kButtonSpacing;
        UIButton* suspectsButton = CreateMainButton(mCanvas, desktopBackground, "SUSPT", buttonPos);
        suspectsButton->SetPressCallback([](UIButton* button){
            Services::GetAudio()->PlaySFX(Services::GetAssets()->LoadAudio("SIDENTER.WAV"));
            printf("Suspects\n");
        });
    }

    // Add "New Email" label.

    // Create subscreens.
    mSearch.Init(mCanvas);
}

void Sidney::Show()
{
    SetActive(true);
}

void Sidney::Hide()
{
    SetActive(false);
}