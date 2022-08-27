#include "SidneySearch.h"

#include "Actor.h"
#include "Texture.h"
#include "UIButton.h"
#include "UICanvas.h"
#include "UIImage.h"
#include "UILabel.h"
#include "UITextInput.h"

UIButton* CreateBasicTextButton(UICanvas* canvas, Actor* parent, const Vector2& pos, const std::string& text)
{
    // Create actor as child of parent.
    Actor* actor = new Actor(Actor::TransformType::RectTransform);
    actor->GetTransform()->SetParent(parent->GetTransform());

    // Add button to actor & canvas.
    UIButton* button = actor->AddComponent<UIButton>();
    canvas->AddWidget(button);

    // Position from top-left, constant size.
    button->GetRectTransform()->SetPivot(0.0f, 1.0f);
    button->GetRectTransform()->SetAnchor(0.0f, 1.0f);
    button->GetRectTransform()->SetAnchoredPosition(pos);
    button->GetRectTransform()->SetSizeDelta(80.0f, 13.0f);

    // Set textures.
    //TODO: Placeholder for now.
    button->SetResizeBasedOnTexture(false);
    button->SetUpTexture(&Texture::Black);

    // Add button text.
    UILabel* buttonLabel = actor->AddComponent<UILabel>();
    canvas->AddWidget(buttonLabel);

    // Font and alignment are constant for now.
    buttonLabel->SetFont(Services::GetAssets()->LoadFont("SID_PDN_10_L.FON"));
    buttonLabel->SetText(text);
    buttonLabel->SetHorizonalAlignment(HorizontalAlignment::Center);
    buttonLabel->SetVerticalAlignment(VerticalAlignment::Bottom);
    return button;
}

void SidneySearch::Init(UICanvas* canvas)
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
        mainMenuButton->SetUpTexture(&Texture::White);

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
                screenNameLabel->SetText("SEARCH");
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

    // Add search bar.
    {
        // Background.
        Actor* searchBarActor = new Actor(Actor::TransformType::RectTransform);
        searchBarActor->GetTransform()->SetParent(mRoot->GetTransform());
        UIImage* searchBarImage = searchBarActor->AddComponent<UIImage>();
        canvas->AddWidget(searchBarImage);

        searchBarImage->SetTexture(&Texture::Black);
        searchBarImage->SetColor(Color32(0, 0, 0, 128));

        searchBarImage->GetRectTransform()->SetPivot(0.0f, 1.0f);
        searchBarImage->GetRectTransform()->SetAnchor(0.0f, 1.0f);
        searchBarImage->GetRectTransform()->SetAnchoredPosition(60.0f, -88.0f);
        searchBarImage->GetRectTransform()->SetSizeDelta(520.0f, 48.0f);

        // Reset button.
        UIButton* resetButton = CreateBasicTextButton(canvas, searchBarActor, Vector2(14.0f, -17.0f), "RESET");
        resetButton->SetPressCallback([](UIButton* button){
            printf("Reset\n");
        });

        // Search button.
        UIButton* searchButton = CreateBasicTextButton(canvas, searchBarActor, Vector2(426.0f, -17.0f), "SEARCH");
        searchButton->SetPressCallback([](UIButton* button){
            printf("Search\n");
        });

        // Text input field.
        Actor* searchInputActor = new Actor(Actor::TransformType::RectTransform);
        searchInputActor->GetTransform()->SetParent(searchBarActor->GetTransform());
        UIImage* searchInputImage = searchInputActor->AddComponent<UIImage>();
        canvas->AddWidget(searchInputImage);

        searchInputImage->SetTexture(&Texture::Black);

        searchInputImage->GetRectTransform()->SetPivot(0.0f, 1.0f);
        searchInputImage->GetRectTransform()->SetAnchor(0.0f, 1.0f);
        searchInputImage->GetRectTransform()->SetAnchoredPosition(100.0f, -17.0f);
        searchInputImage->GetRectTransform()->SetSizeDelta(320.0f, 15.0f);

        mTextInput = searchInputActor->AddComponent<UITextInput>();
        canvas->AddWidget(mTextInput);
        mTextInput->SetFont(Services::GetAssets()->LoadFont("F_TIMES.FON"));
        mTextInput->SetVerticalAlignment(VerticalAlignment::Bottom);
        mTextInput->SetText("");

        // Create text input field caret.
        Actor* caretActor = new Actor(Actor::TransformType::RectTransform);
        caretActor->GetTransform()->SetParent(searchInputActor->GetTransform());
        UIImage* caretImage = caretActor->AddComponent<UIImage>();
        canvas->AddWidget(caretImage);

        caretImage->SetTexture(&Texture::White);

        caretImage->GetRectTransform()->SetPivot(0.0f, 0.0f);
        caretImage->GetRectTransform()->SetAnchor(0.0f, 1.0f);
        caretImage->GetRectTransform()->SetSizeDelta(1.0f, 15.0f);

        mTextInput->SetCaret(caretImage);
        mTextInput->SetCaretBlinkInterval(0.5f);
    }

    // Add navigation bar.
    {
        // Background.
        Actor* navBarActor = new Actor(Actor::TransformType::RectTransform);
        navBarActor->GetTransform()->SetParent(mRoot->GetTransform());
        UIImage* navBarImage = navBarActor->AddComponent<UIImage>();
        canvas->AddWidget(navBarImage);

        navBarImage->SetTexture(&Texture::White);

        navBarImage->GetRectTransform()->SetPivot(0.0f, 1.0f);
        navBarImage->GetRectTransform()->SetAnchor(0.0f, 1.0f);
        navBarImage->GetRectTransform()->SetAnchoredPosition(60.0f, -390.0f);
        navBarImage->GetRectTransform()->SetSizeDelta(520.0f, 28.0f);

        // Back button.
        UIButton* backButton = CreateBasicTextButton(canvas, navBarActor, Vector2(63.0f, -8.0f), "BACK");
        backButton->SetPressCallback([](UIButton* button){
            printf("Back\n");
        });

        // Forward button.
        UIButton* fwdButton = CreateBasicTextButton(canvas, navBarActor, Vector2(155.0f, -8.0f), "FORWARD");
        fwdButton->SetPressCallback([](UIButton* button){
            printf("Forward\n");
        });
    }

    // Add search result text area.
    Actor* resultsBackgroundActor = new Actor(Actor::TransformType::RectTransform);
    resultsBackgroundActor->GetTransform()->SetParent(mRoot->GetTransform());
    UIImage* resultsBackgroundImage = resultsBackgroundActor->AddComponent<UIImage>();
    canvas->AddWidget(resultsBackgroundImage);

    resultsBackgroundImage->SetTexture(&Texture::Black);
    resultsBackgroundImage->SetColor(Color32(0, 0, 0, 128)); // Black Semi-Transparent

    resultsBackgroundImage->GetRectTransform()->SetPivot(0.0f, 1.0f);
    resultsBackgroundImage->GetRectTransform()->SetAnchor(0.0f, 1.0f);
    resultsBackgroundImage->GetRectTransform()->SetAnchoredPosition(60.0f, -136.0f);
    resultsBackgroundImage->GetRectTransform()->SetSizeDelta(520.0f, 254.0f);

    Hide();
}

void SidneySearch::Show()
{
    mRoot->SetActive(true);
    mTextInput->Focus();
}

void SidneySearch::Hide()
{
    mRoot->SetActive(false);
}