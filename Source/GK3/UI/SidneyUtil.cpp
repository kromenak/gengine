#include "SidneyUtil.h"

#include "Actor.h"
#include "TextAsset.h"
#include "Texture.h"
#include "UIButton.h"
#include "UIImage.h"
#include "UILabel.h"

UIButton* SidneyUtil::CreateTextButton(Actor* parent, const std::string& text, const std::string& font,
                                       const Vector2& pivotAndAnchor, const Vector2& position, const Vector2& size)
{
    // Create actor as child of parent.
    Actor* actor = new Actor(TransformType::RectTransform);
    actor->GetTransform()->SetParent(parent->GetTransform());

    // Add button to Actor.
    UIButton* button = actor->AddComponent<UIButton>();

    // Position and size appropriately.
    button->GetRectTransform()->SetPivot(pivotAndAnchor);
    button->GetRectTransform()->SetAnchor(pivotAndAnchor);
    button->GetRectTransform()->SetAnchoredPosition(position);
    button->GetRectTransform()->SetSizeDelta(size);

    // Set textures.
    //TODO: Placeholder for now.
    button->SetResizeBasedOnTexture(false);
    button->SetUpTexture(&Texture::Black);

    // Add button text.
    UILabel* buttonLabel = actor->AddComponent<UILabel>();

    // Set font and alignment.
    buttonLabel->SetFont(Services::GetAssets()->LoadFont(font));
    buttonLabel->SetHorizonalAlignment(HorizontalAlignment::Center);
    buttonLabel->SetVerticalAlignment(VerticalAlignment::Bottom);

    // Set text.
    buttonLabel->SetText(text);
    return button;
}

void SidneyUtil::CreateMenuBar(Actor* parent, const std::string& screenName)
{
    // Bar that stretches across entire screen.
    {
        Actor* menuBarActor = new Actor(TransformType::RectTransform);
        menuBarActor->GetTransform()->SetParent(parent->GetTransform());
        UIImage* menuBarImage = menuBarActor->AddComponent<UIImage>();

        menuBarImage->SetTexture(Services::GetAssets()->LoadTexture("S_BAR_STRETCH.BMP"), true);
        menuBarImage->SetRenderMode(UIImage::RenderMode::Tiled);

        menuBarImage->GetRectTransform()->SetPivot(1.0f, 1.0f); // Top-Right
        menuBarImage->GetRectTransform()->SetAnchorMin(0.0f, 1.0f); // Anchor to Top, Stretch Horizontally
        menuBarImage->GetRectTransform()->SetAnchorMax(1.0f, 1.0f);
        menuBarImage->GetRectTransform()->SetAnchoredPosition(0.0f, -25.0f);
    }

    // Bar that extends from top-right, used to give enough height for the screen name label.
    {
        Actor* menuBarTopActor = new Actor(TransformType::RectTransform);
        menuBarTopActor->GetTransform()->SetParent(parent->GetTransform());
        UIImage* menuBarTopImage = menuBarTopActor->AddComponent<UIImage>();

        menuBarTopImage->SetTexture(Services::GetAssets()->LoadTexture("S_BAR_TOPSTRIP_LR.BMP"), true);
        menuBarTopImage->SetRenderMode(UIImage::RenderMode::Tiled);

        menuBarTopImage->GetRectTransform()->SetPivot(1.0f, 1.0f); // Top-Right
        menuBarTopImage->GetRectTransform()->SetAnchor(1.0f, 1.0f); // Anchor to Top-Right
        menuBarTopImage->GetRectTransform()->SetAnchoredPosition(0.0f, -16.0f);
        menuBarTopImage->GetRectTransform()->SetSizeDeltaX(100.0f);

        // Triangle bit that slopes downward.
        {
            Actor* menuBarAngleActor = new Actor(TransformType::RectTransform);
            menuBarAngleActor->GetTransform()->SetParent(menuBarTopActor->GetTransform());
            UIImage* menuBarAngleImage = menuBarAngleActor->AddComponent<UIImage>();

            menuBarAngleImage->SetTexture(Services::GetAssets()->LoadTexture("S_BAR_TOPANGLE_LR.BMP"), true);

            menuBarAngleImage->GetRectTransform()->SetPivot(1.0f, 1.0f); // Top-Right
            menuBarAngleImage->GetRectTransform()->SetAnchor(0.0f, 1.0f); // Anchor to Top-Left
            menuBarAngleImage->GetRectTransform()->SetAnchoredPosition(0.0f, 0.0f);
        }

        // Screen name label.
        {
            Actor* screenNameActor = new Actor(TransformType::RectTransform);
            screenNameActor->GetTransform()->SetParent(menuBarTopActor->GetTransform());
            UILabel* screenNameLabel = screenNameActor->AddComponent<UILabel>();

            screenNameLabel->SetFont(Services::GetAssets()->LoadFont("SID_EMB_18.FON"));
            screenNameLabel->SetText(screenName);
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

const Localizer& SidneyUtil::GetMainScreenLocalizer()
{
    static Localizer localizer("SIDNEY.TXT", "Main Screen");
    return localizer;
}

const Localizer& SidneyUtil::GetAddDataLocalizer()
{
    static Localizer localizer("SIDNEY.TXT", "AddData Screen");
    return localizer;
}