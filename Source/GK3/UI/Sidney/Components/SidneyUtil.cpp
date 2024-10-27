#include "SidneyUtil.h"

#include "Actor.h"
#include "AssetManager.h"
#include "SidneyButton.h"
#include "TextAsset.h"
#include "Texture.h"
#include "UIButton.h"
#include "UIImage.h"
#include "UILabel.h"
#include "UINineSlice.h"

Color32 SidneyUtil::TransBgColor = Color32(0, 0, 0, 128);

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
    buttonLabel->SetFont(gAssetManager.LoadFont(font));
    buttonLabel->SetHorizonalAlignment(HorizontalAlignment::Center);
    buttonLabel->SetVerticalAlignment(VerticalAlignment::Bottom);

    // Set text.
    buttonLabel->SetText(text);
    return button;
}

Actor* SidneyUtil::CreateBackground(Actor* parent)
{
    // Create actor that is child of the parent.
    Actor* backgroundActor = new Actor(TransformType::RectTransform);
    backgroundActor->GetTransform()->SetParent(parent->GetTransform());

    // Add the background image and size to fit.
    UIImage* backgroundImage = backgroundActor->AddComponent<UIImage>();
    backgroundImage->SetTexture(gAssetManager.LoadTexture("S_BKGND.BMP"), true);

    // Receive input to avoid sending inputs to main screen below this screen.
    backgroundImage->SetReceivesInput(true);
    return backgroundActor;
}

void SidneyUtil::CreateMainMenuButton(Actor* parent, std::function<void()> pressCallback)
{
    SidneyButton* button = new SidneyButton(parent);
    button->SetFont(gAssetManager.LoadFont("SID_TEXT_18.FON"));
    button->SetText(GetMainScreenLocalizer().GetText("HomeButton"));
    button->SetWidth(94.0f);

    button->SetPressCallback(pressCallback);
    button->SetPressAudio(gAssetManager.LoadAudio("SIDEXIT.WAV"));

    button->GetRectTransform()->SetPivot(1.0f, 0.0f); // Bottom-Right
    button->GetRectTransform()->SetAnchor(1.0f, 0.0f); // Bottom-Right
    button->GetRectTransform()->SetAnchoredPosition(-10.0f, 10.0f); // 10x10 offset from Bottom-Right
}

Actor* SidneyUtil::CreateMenuBar(Actor* parent, const std::string& screenName, float labelWidth)
{
    // Bar that stretches across entire screen.
    Actor* menuBarActor = new Actor(TransformType::RectTransform);
    menuBarActor->GetTransform()->SetParent(parent->GetTransform());
    {
        UIImage* menuBarImage = menuBarActor->AddComponent<UIImage>();

        menuBarImage->SetTexture(gAssetManager.LoadTexture("S_BAR_STRETCH.BMP"), true);
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

        menuBarTopImage->SetTexture(gAssetManager.LoadTexture("S_BAR_TOPSTRIP_LR.BMP"), true);
        menuBarTopImage->SetRenderMode(UIImage::RenderMode::Tiled);

        menuBarTopImage->GetRectTransform()->SetPivot(1.0f, 1.0f); // Top-Right
        menuBarTopImage->GetRectTransform()->SetAnchor(1.0f, 1.0f); // Anchor to Top-Right
        menuBarTopImage->GetRectTransform()->SetAnchoredPosition(0.0f, -16.0f);
        menuBarTopImage->GetRectTransform()->SetSizeDeltaX(labelWidth);

        // Triangle bit that slopes downward.
        {
            Actor* menuBarAngleActor = new Actor(TransformType::RectTransform);
            menuBarAngleActor->GetTransform()->SetParent(menuBarTopActor->GetTransform());
            UIImage* menuBarAngleImage = menuBarAngleActor->AddComponent<UIImage>();

            menuBarAngleImage->SetTexture(gAssetManager.LoadTexture("S_BAR_TOPANGLE_LR.BMP"), true);

            menuBarAngleImage->GetRectTransform()->SetPivot(1.0f, 1.0f); // Top-Right
            menuBarAngleImage->GetRectTransform()->SetAnchor(0.0f, 1.0f); // Anchor to Top-Left
            menuBarAngleImage->GetRectTransform()->SetAnchoredPosition(0.0f, 0.0f);
        }

        // Screen name label.
        {
            Actor* screenNameActor = new Actor(TransformType::RectTransform);
            screenNameActor->GetTransform()->SetParent(menuBarTopActor->GetTransform());
            UILabel* screenNameLabel = screenNameActor->AddComponent<UILabel>();

            screenNameLabel->SetFont(gAssetManager.LoadFont("SID_EMB_18.FON"));
            screenNameLabel->SetText(screenName);
            screenNameLabel->SetHorizonalAlignment(HorizontalAlignment::Right);
            screenNameLabel->SetVerticalAlignment(VerticalAlignment::Top);
            screenNameLabel->SetMasked(true);

            screenNameLabel->GetRectTransform()->SetPivot(1.0f, 1.0f); // Top-Right
            screenNameLabel->GetRectTransform()->SetAnchor(1.0f, 1.0f); // Top-Right
            screenNameLabel->GetRectTransform()->SetAnchoredPosition(-4.0f, -1.0f); // Nudge a bit to get right positioning
            screenNameLabel->GetRectTransform()->SetSizeDelta(labelWidth, 18.0f);
        }
    }
    return menuBarActor;
}

SidneyButton* SidneyUtil::CreateBigButton(Actor* parent)
{
    SidneyButton* button = new SidneyButton(parent);
    button->SetFont(gAssetManager.LoadFont("SID_TEXT_18.FON"), gAssetManager.LoadFont("SID_TEXT_18_UL.FON"));
    return button;
}

const UINineSliceParams& SidneyUtil::GetGrayBoxParams(const Color32& centerColor)
{
    static UINineSliceParams params;
    if(params.topTexture == nullptr)
    {
        params.topLeftTexture = gAssetManager.LoadTexture("S_BOX_CORNER_TL.BMP");
        params.topRightTexture = gAssetManager.LoadTexture("S_BOX_CORNER_TR.BMP");
        params.bottomLeftTexture = gAssetManager.LoadTexture("S_BOX_CORNER_BL.BMP");
        params.bottomRightTexture = gAssetManager.LoadTexture("S_BOX_CORNER_BR.BMP");
        params.topLeftColor = params.topRightColor = params.bottomLeftColor = params.bottomRightColor = Color32::Clear;

        params.leftTexture = params.rightTexture = gAssetManager.LoadTexture("S_BOX_SIDE.BMP");
        params.bottomTexture = params.topTexture = gAssetManager.LoadTexture("S_BOX_TOP.BMP");
    }
    params.centerColor = centerColor;
    return params;
}

const UINineSliceParams& SidneyUtil::GetGoldBoxParams(const Color32& centerColor)
{
    static UINineSliceParams params;
    if(params.topTexture == nullptr)
    {
        params.topLeftTexture = gAssetManager.LoadTexture("S_BOX_CORNER_TL_L.BMP");
        params.topRightTexture = gAssetManager.LoadTexture("S_BOX_CORNER_TR_L.BMP");
        params.bottomLeftTexture = gAssetManager.LoadTexture("S_BOX_CORNER_BL_L.BMP");
        params.bottomRightTexture = gAssetManager.LoadTexture("S_BOX_CORNER_BR_L.BMP");
        params.topLeftColor = params.topRightColor = params.bottomLeftColor = params.bottomRightColor = Color32::Clear;

        params.leftTexture = params.rightTexture = gAssetManager.LoadTexture("S_BOX_SIDE_L.BMP");
        params.bottomTexture = params.topTexture = gAssetManager.LoadTexture("S_BOX_TOP_L.BMP");
    }
    params.centerColor = centerColor;
    return params;
}

const Localizer& SidneyUtil::GetMainScreenLocalizer()
{
    static Localizer localizer("SIDNEY.TXT", "Main Screen");
    return localizer;
}

const Localizer& SidneyUtil::GetAnalyzeLocalizer()
{
    static Localizer localizer("SIDNEY.TXT", "Analyze Screen");
    return localizer;
}

const Localizer& SidneyUtil::GetAddDataLocalizer()
{
    static Localizer localizer("SIDNEY.TXT", "AddData Screen");
    return localizer;
}

const Localizer& SidneyUtil::GetMakeIdLocalizer()
{
    static Localizer localizer("SIDNEY.TXT", "MakeID Screen");
    return localizer;
}