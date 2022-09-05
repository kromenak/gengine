#include "SidneyEmail.h"

#include "Texture.h"
#include "UIButton.h"
#include "UICanvas.h"
#include "UIImage.h"
#include "UILabel.h"

void SidneyEmail::Init(UICanvas* canvas)
{
    // Add background. This will also be the root for this screen.
    mRoot = new Actor(Actor::TransformType::RectTransform);
    mRoot->GetTransform()->SetParent(canvas->GetRectTransform());
    UIImage* backgroundImage = mRoot->AddComponent<UIImage>();
    canvas->AddWidget(backgroundImage);
    backgroundImage->SetTexture(Services::GetAssets()->LoadTexture("S_BKGND.BMP"), true);

    // Receive input to avoid sending inputs to main screen below this screen.
    backgroundImage->SetReceivesInput(true);

    // Add main menu button.
    {
        Actor* mainMenuButtonActor = new Actor(Actor::TransformType::RectTransform);
        mainMenuButtonActor->GetTransform()->SetParent(mRoot->GetTransform());
        UIButton* mainMenuButton = mainMenuButtonActor->AddComponent<UIButton>();
        canvas->AddWidget(mainMenuButton);

        mainMenuButton->GetRectTransform()->SetPivot(1.0f, 0.0f); // Bottom-Right
        mainMenuButton->GetRectTransform()->SetAnchor(1.0f, 0.0f); // Bottom-Right
        mainMenuButton->GetRectTransform()->SetAnchoredPosition(-10.0f, 10.0f); // 10x10 offset from Bottom-Right
        mainMenuButton->GetRectTransform()->SetSizeDelta(94.0f, 18.0f);

        //TODO: Exit button uses a stretch-based image solution (kind of like 9-slice, more like 3-slice).
        mainMenuButton->SetResizeBasedOnTexture(false);
        mainMenuButton->SetUpTexture(&Texture::Black);

        mainMenuButton->SetPressCallback([&](UIButton* button){
            Services::GetAudio()->PlaySFX(Services::GetAssets()->LoadAudio("SIDEXIT.WAV"));
            Hide();
        });

        // Add exit button text.
        UILabel* mainMenuLabel = mainMenuButtonActor->AddComponent<UILabel>();
        canvas->AddWidget(mainMenuLabel);
        mainMenuLabel->SetFont(Services::GetAssets()->LoadFont("SID_TEXT_18.FON"));
        mainMenuLabel->SetText("MAIN MENU");
        mainMenuLabel->SetHorizonalAlignment(HorizontalAlignment::Center);
        mainMenuLabel->SetVerticalAlignment(VerticalAlignment::Center);
    }

    // Add menu bar.
    {
        // Bar that stretches across entire screen.
        {
            Actor* menuBarActor = new Actor(Actor::TransformType::RectTransform);
            menuBarActor->GetTransform()->SetParent(mRoot->GetTransform());
            UIImage* menuBarImage = menuBarActor->AddComponent<UIImage>();
            canvas->AddWidget(menuBarImage);

            menuBarImage->SetTexture(Services::GetAssets()->LoadTexture("S_BAR_STRETCH.BMP"), true);
            menuBarImage->SetRenderMode(UIImage::RenderMode::Tiled);

            menuBarImage->GetRectTransform()->SetPivot(1.0f, 1.0f); // Top-Right
            menuBarImage->GetRectTransform()->SetAnchorMin(0.0f, 1.0f); // Anchor to Top, Stretch Horizontally
            menuBarImage->GetRectTransform()->SetAnchorMax(1.0f, 1.0f);
            menuBarImage->GetRectTransform()->SetAnchoredPosition(0.0f, -25.0f);
        }

        // Bar that extends from top-right, used to give enough height for the screen name label.
        {
            Actor* menuBarTopActor = new Actor(Actor::TransformType::RectTransform);
            menuBarTopActor->GetTransform()->SetParent(mRoot->GetTransform());
            UIImage* menuBarTopImage = menuBarTopActor->AddComponent<UIImage>();
            canvas->AddWidget(menuBarTopImage);

            menuBarTopImage->SetTexture(Services::GetAssets()->LoadTexture("S_BAR_TOPSTRIP_LR.BMP"), true);
            menuBarTopImage->SetRenderMode(UIImage::RenderMode::Tiled);

            menuBarTopImage->GetRectTransform()->SetPivot(1.0f, 1.0f); // Top-Right
            menuBarTopImage->GetRectTransform()->SetAnchor(1.0f, 1.0f); // Anchor to Top-Right
            menuBarTopImage->GetRectTransform()->SetAnchoredPosition(0.0f, -16.0f);
            menuBarTopImage->GetRectTransform()->SetSizeDeltaX(100.0f);

            // Triangle bit that slopes downward.
            {
                Actor* menuBarAngleActor = new Actor(Actor::TransformType::RectTransform);
                menuBarAngleActor->GetTransform()->SetParent(menuBarTopActor->GetTransform());
                UIImage* menuBarAngleImage = menuBarAngleActor->AddComponent<UIImage>();
                canvas->AddWidget(menuBarAngleImage);

                menuBarAngleImage->SetTexture(Services::GetAssets()->LoadTexture("S_BAR_TOPANGLE_LR.BMP"), true);

                menuBarAngleImage->GetRectTransform()->SetPivot(1.0f, 1.0f); // Top-Right
                menuBarAngleImage->GetRectTransform()->SetAnchor(0.0f, 1.0f); // Anchor to Top-Left
                menuBarAngleImage->GetRectTransform()->SetAnchoredPosition(0.0f, 0.0f);
            }

            // Screen name label.
            {
                Actor* screenNameActor = new Actor(Actor::TransformType::RectTransform);
                screenNameActor->GetTransform()->SetParent(menuBarTopActor->GetTransform());
                UILabel* screenNameLabel = screenNameActor->AddComponent<UILabel>();
                canvas->AddWidget(screenNameLabel);

                screenNameLabel->SetFont(Services::GetAssets()->LoadFont("SID_EMB_18.FON"));
                screenNameLabel->SetText("E-MAIL");
                screenNameLabel->SetHorizonalAlignment(HorizontalAlignment::Right);
                screenNameLabel->SetVerticalAlignment(VerticalAlignment::Top);
                screenNameLabel->SetMasked(true);

                screenNameLabel->GetRectTransform()->SetPivot(1.0f, 1.0f); // Top-Right
                screenNameLabel->GetRectTransform()->SetAnchor(1.0f, 1.0f); // Top-Right
                screenNameLabel->GetRectTransform()->SetAnchoredPosition(-4.0f, -1.0f); // Nudge a bit to get right positioning
                screenNameLabel->GetRectTransform()->SetSizeDelta(100.0f, 18.0f);
            }
        }
    }

    // Create email list window.
    {

    }
    
    // Hide by default.
    Hide();
}

void SidneyEmail::Show()
{
    mRoot->SetActive(true);
}

void SidneyEmail::Hide()
{
    mRoot->SetActive(false);
}