#include "SidneyEmail.h"

#include "SidneyUtil.h"
#include "Texture.h"
#include "UIButton.h"
#include "UIImage.h"
#include "UILabel.h"

void SidneyEmail::Init(Actor* parent)
{
    // Add background. This will also be the root for this screen.
    mRoot = new Actor(TransformType::RectTransform);
    mRoot->GetTransform()->SetParent(parent->GetTransform());
    UIImage* backgroundImage = mRoot->AddComponent<UIImage>();
    backgroundImage->SetTexture(Services::GetAssets()->LoadTexture("S_BKGND.BMP"), true);

    // Receive input to avoid sending inputs to main screen below this screen.
    backgroundImage->SetReceivesInput(true);

    // Add main menu button.
    {
        Actor* mainMenuButtonActor = new Actor(TransformType::RectTransform);
        mainMenuButtonActor->GetTransform()->SetParent(mRoot->GetTransform());
        UIButton* mainMenuButton = mainMenuButtonActor->AddComponent<UIButton>();

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
        mainMenuLabel->SetFont(Services::GetAssets()->LoadFont("SID_TEXT_18.FON"));
        mainMenuLabel->SetText("MAIN MENU");
        mainMenuLabel->SetHorizonalAlignment(HorizontalAlignment::Center);
        mainMenuLabel->SetVerticalAlignment(VerticalAlignment::Center);
    }

    // Add menu bar.
    SidneyUtil::CreateMenuBar(mRoot, "E-MAIL");

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