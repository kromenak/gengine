#include "SidneyUtil.h"

#include "ActionManager.h"
#include "Actor.h"
#include "AssetManager.h"
#include "GameProgress.h"
#include "GK3UI.h"
#include "Sidney.h"
#include "SidneyButton.h"
#include "TextAsset.h"
#include "Texture.h"
#include "UIButton.h"
#include "UIImage.h"
#include "UILabel.h"
#include "UINineSlice.h"
#include "UIUtil.h"

Color32 SidneyUtil::TransBgColor = Color32(0, 0, 0, 128);
Color32 SidneyUtil::VeryTransBgColor = Color32(0, 0, 0, 96);

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

void SidneyUtil::CreateMainMenuButton(Actor* parent, const std::function<void()>& pressCallback)
{
    SidneyButton* button = new SidneyButton("MainMenuButton", parent);
    button->SetFont(gAssetManager.LoadFont("SID_TEXT_18.FON"));
    button->SetText(GetMainScreenLocalizer().GetText("HomeButton"));
    button->SetWidth(94.0f);

    button->SetPressCallback(pressCallback);
    button->SetPressAudio(gAssetManager.LoadAudio("SIDEXIT.WAV"));

    button->GetRectTransform()->SetPivot(1.0f, 0.0f); // Bottom-Right
    button->GetRectTransform()->SetAnchor(1.0f, 0.0f); // Bottom-Right
    button->GetRectTransform()->SetAnchoredPosition(-10.0f, 10.0f); // 10x10 offset from Bottom-Right
}

UIButton* SidneyUtil::CreateCloseWindowButton(Actor* parent, const std::function<void()>& pressCallback)
{
    UIButton* closeButton = UI::CreateWidgetActor<UIButton>("CloseButton", parent);
    closeButton->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
    closeButton->GetRectTransform()->SetAnchoredPosition(2.0f, -2.0f);

    closeButton->SetUpTexture(gAssetManager.LoadTexture("CLOSEWIN_UP.BMP"));
    closeButton->SetDownTexture(gAssetManager.LoadTexture("CLOSEWIN_DOWN.BMP"));
    closeButton->SetHoverTexture(gAssetManager.LoadTexture("CLOSEWIN_HOVER.BMP"));

    closeButton->SetPressCallback([pressCallback](UIButton* button) {
        pressCallback();
    });
    return closeButton;
}

SidneyButton* SidneyUtil::CreateBigButton(Actor* parent)
{
    SidneyButton* button = new SidneyButton(parent);
    button->SetFont(gAssetManager.LoadFont("SID_TEXT_18.FON"), gAssetManager.LoadFont("SID_TEXT_18_UL.FON"));
    return button;
}

SidneyButton* SidneyUtil::CreateSmallButton(Actor* parent)
{
    SidneyButton* button = new SidneyButton(parent);
    button->SetFont(gAssetManager.LoadFont("SID_PDN_10_L.FON"), gAssetManager.LoadFont("SID_PDN_10_UL.FON"));
    button->SetWidth(80.0f);
    button->SetHeight(13.0f);
    return button;
}

FilePreviewWindow SidneyUtil::CreateFilePreviewWindow(Actor* parent)
{
    // Add window border.
    UINineSlice* border = UI::CreateWidgetActor<UINineSlice>("FilePreviewWindow", parent, SidneyUtil::GetGrayBoxParams(SidneyUtil::TransBgColor));
    border->GetRectTransform()->SetSizeDelta(153.0f, 167.0f);

    // Add one line for the box header.
    UIImage* boxHeaderDividerImage = UI::CreateWidgetActor<UIImage>("Divider", border);
    boxHeaderDividerImage->SetTexture(gAssetManager.LoadTexture("S_BOX_TOP.BMP"), true);
    boxHeaderDividerImage->GetRectTransform()->SetAnchor(AnchorPreset::TopStretch);
    boxHeaderDividerImage->GetRectTransform()->SetAnchoredPosition(0.0f, -17.0f);
    boxHeaderDividerImage->GetRectTransform()->SetSizeDeltaX(0.0f);

    // Add header label.
    UILabel* headerLabel = UI::CreateWidgetActor<UILabel>("HeaderLabel", border);
    headerLabel->SetFont(gAssetManager.LoadFont("SID_TEXT_14.FON"));
    headerLabel->SetText("");
    headerLabel->SetHorizonalAlignment(HorizontalAlignment::Right);
    headerLabel->SetVerticalAlignment(VerticalAlignment::Top);
    headerLabel->SetMasked(true);

    headerLabel->GetRectTransform()->SetAnchor(AnchorPreset::TopRight);
    headerLabel->GetRectTransform()->SetAnchoredPosition(-8.0f, -3.0f);
    headerLabel->GetRectTransform()->SetSizeDeltaY(14.0f);

    // Add item image.
    UIImage* image = UI::CreateWidgetActor<UIImage>("Image", border);
    image->GetRectTransform()->SetAnchoredPosition(0.0f, -8.0f);
    image->GetRectTransform()->SetSizeDelta(94.0f, 94.0f);

    FilePreviewWindow window;
    window.root = border->GetOwner();
    window.image = image;
    window.header = headerLabel;
    return window;
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

const Localizer& SidneyUtil::GetSearchLocalizer()
{
    static Localizer localizer("SIDNEY.TXT", "Search Screen");
    return localizer;
}

const Localizer& SidneyUtil::GetEmailLocalizer()
{
    static Localizer localizer("SIDNEY.TXT", "EMail Screen");
    return localizer;
}

const Localizer& SidneyUtil::GetAnalyzeLocalizer()
{
    static Localizer localizer("SIDNEY.TXT", "Analyze Screen");
    return localizer;
}

const Localizer& SidneyUtil::GetTranslateLocalizer()
{
    static Localizer localizer("SIDNEY.TXT", "Translate Screen");
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

const Localizer& SidneyUtil::GetSuspectsLocalizer()
{
    static Localizer localizer("SIDNEY.TXT", "Suspects Screen");
    return localizer;
}

int SidneyUtil::GetCurrentLSRStep()
{
    int step = 0;
    if(gGameProgress.GetFlag("Aquarius"))
    {
        ++step;
    }
    if(gGameProgress.GetFlag("Pisces"))
    {
        ++step;
    }
    if(gGameProgress.GetFlag("Aries"))
    {
        ++step;
    }
    if(gGameProgress.GetFlag("Taurus"))
    {
        ++step;
    }
    if(gGameProgress.GetFlag("Gemini"))
    {
        ++step;
    }
    if(gGameProgress.GetFlag("Cancer"))
    {
        ++step;
    }
    if(gGameProgress.GetFlag("Leo"))
    {
        ++step;
    }
    if(gGameProgress.GetFlag("Virgo"))
    {
        ++step;
    }
    if(gGameProgress.GetFlag("Libra"))
    {
        ++step;
    }
    if(gGameProgress.GetFlag("Scorpio"))
    {
        ++step;
    }
    if(gGameProgress.GetFlag("Ophiuchus"))
    {
        ++step;
    }
    if(gGameProgress.GetFlag("Sagittarius"))
    {
        ++step;
    }
    if(gGameProgress.GetFlag("Capricorn"))
    {
        ++step;
    }
    return step;
}

void SidneyUtil::UpdateLSRState()
{
    // Some game logic uses this variable to trigger time advances. So, keep it up to date!
    gGameProgress.SetGameVariable("LSRState", GetCurrentLSRStep());
}

void SidneyUtil::CheckForceExitSidney307A()
{
    // Day 3, 7AM has several timeblock end conditions that can be done in any order. Several of them are in Sidney.
    // If it seems like doing this action did end the timeblock, force exit Sidney so the timeblock ends.
    if(gActionManager.IsActionAllowed("SCENE", "ENTER", "END_TIME_BLOCK"))
    {
        gGK3UI.GetSidney()->Hide();
    }
}
