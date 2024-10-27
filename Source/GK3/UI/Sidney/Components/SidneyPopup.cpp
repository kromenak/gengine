#include "SidneyPopup.h"

#include "AssetManager.h"
#include "RectTransform.h"
#include "SidneyButton.h"
#include "SidneyUtil.h"
#include "Texture.h"
#include "UIButton.h"
#include "UICanvas.h"
#include "UIImage.h"
#include "UINineSlice.h"
#include "UIUtil.h"

SidneyPopup::SidneyPopup(Actor* parent) : Actor("Sidney Popup", TransformType::RectTransform)
{
    // Create popup root, which covers whole screen and acts as an input blocker while the message is up.
    {
        // Set as child of parent object passed in.
        GetTransform()->SetParent(parent->GetTransform());

        // Add a UICanvas, so we can force this to display above other things.
        AddComponent<UICanvas>(1);

        // Stretch to fill the parent container and add a button.
        // This causes us to act as an invisible input blocker over the parent area.
        GetComponent<RectTransform>()->SetAnchor(AnchorPreset::CenterStretch);
        AddComponent<UIButton>();
    }

    // Add the window itself.
    {
        // A child of the this actor.
        mWindow = new Actor(TransformType::RectTransform);
        mWindow->GetTransform()->SetParent(GetTransform());

        // Add a border around the edge.
        UINineSlice* border = mWindow->AddComponent<UINineSlice>(SidneyUtil::GetGrayBoxParams(Color32::Black));

        // Default to centered on-screen at a certain size.
        border->GetRectTransform()->SetSizeDelta(250.0f, 172.0f);
        border->GetRectTransform()->SetAnchor(AnchorPreset::Center);
    }

    // Add message label.
    {
        Actor* messageActor = new Actor(TransformType::RectTransform);
        messageActor->GetTransform()->SetParent(mWindow->GetTransform());

        mMessage = messageActor->AddComponent<UILabel>();
        mMessage->SetFont(gAssetManager.LoadFont("SID_TEXT_14.FON"));
        mMessage->SetHorizonalAlignment(HorizontalAlignment::Left);
        mMessage->SetHorizontalOverflow(HorizontalOverflow::Wrap);
        mMessage->SetVerticalAlignment(VerticalAlignment::Top);

        mMessage->GetRectTransform()->SetAnchor(AnchorPreset::CenterStretch);
        mMessage->GetRectTransform()->SetSizeDelta(-15.0f, -15.0f);
    }

    // Add image element.
    {
        mImage = UIUtil::NewUIActorWithWidget<UIImage>(mWindow);
        mImage->SetTexture(&Texture::White, true);

        mImage->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
        mImage->GetRectTransform()->SetAnchoredPosition(10.0f, -10.0f);
    }

    // Add OK button.
    {
        mOKButton = new SidneyButton(mWindow);
        mOKButton->SetFont(gAssetManager.LoadFont("SID_PDN_10_L.FON"));
        mOKButton->SetText(SidneyUtil::GetAnalyzeLocalizer().GetText("OKButton"));
        mOKButton->SetWidth(80.0f);
        mOKButton->SetHeight(13.0f);

        mOKButton->GetRectTransform()->SetAnchor(AnchorPreset::Bottom);
        mOKButton->GetRectTransform()->SetAnchoredPosition(0.0f, 8.0f);

        // Hide on button press.
        mOKButton->SetPressCallback([this](){
            SetActive(false);
        });
    }

    // Add YES button.
    {
        mYesButton = new SidneyButton(mWindow);
        mYesButton->SetFont(gAssetManager.LoadFont("SID_PDN_10_L.FON"));
        mYesButton->SetText(SidneyUtil::GetAnalyzeLocalizer().GetText("YesButton"));
        mYesButton->SetWidth(80.0f);
        mYesButton->SetHeight(13.0f);

        mYesButton->GetRectTransform()->SetAnchor(AnchorPreset::Bottom);
        mYesButton->GetRectTransform()->SetAnchoredPosition(-48.0f, 8.0f);

        // Hide on button press.
        mYesButton->SetPressCallback([this](){
            printf("Yes\n");
            SetActive(false);
        });
    }

    // Add NO button.
    {
        mNoButton = new SidneyButton(mWindow);
        mNoButton->SetFont(gAssetManager.LoadFont("SID_PDN_10_L.FON"));
        mNoButton->SetText(SidneyUtil::GetAnalyzeLocalizer().GetText("NoButton"));
        mNoButton->SetWidth(80.0f);
        mNoButton->SetHeight(13.0f);

        mNoButton->GetRectTransform()->SetAnchor(AnchorPreset::Bottom);
        mNoButton->GetRectTransform()->SetAnchoredPosition(48.0f, 8.0f);

        // Hide on button press.
        mNoButton->SetPressCallback([this](){
            SetActive(false);
        });
    }

    // Hide by default.
    SetActive(false);
}

void SidneyPopup::ResetToDefaults()
{
    // Reset to initial position and size.
    mWindow->GetComponent<RectTransform>()->SetAnchoredPosition(0.0f, 0.0f);
    mWindow->GetComponent<RectTransform>()->SetSizeDelta(250.0f, 172.0f);

    // Reset message positioning.
    mMessage->GetRectTransform()->SetAnchor(AnchorPreset::CenterStretch);
    mMessage->GetRectTransform()->SetAnchoredPosition(0.0f, 0.0f);
    mMessage->GetRectTransform()->SetSizeDelta(-15.0f, -15.0f);

    // Reset to left alignment for the message.
    mMessage->SetHorizonalAlignment(HorizontalAlignment::Left);

    // Assume image will be hidden by default.
    mImage->SetTexture(nullptr);
    mImage->SetEnabled(false);
}

void SidneyPopup::SetWindowPosition(const Vector2& position)
{
    mWindow->GetComponent<RectTransform>()->SetAnchoredPosition(position);
}

void SidneyPopup::SetText(const std::string& message)
{
    mMessage->SetText(SidneyUtil::GetAnalyzeLocalizer().GetText(message));

    // Also adjust height to fit the size of the message.
    // +30 to account for margins around text, +6 for extra buffer between message and button.
    mWindow->GetComponent<RectTransform>()->SetSizeDeltaY(mMessage->GetTextHeight() + 30 + 6);
}

void SidneyPopup::SetTextAlignment(HorizontalAlignment textAlignment)
{
    mMessage->SetHorizonalAlignment(textAlignment);
}

void SidneyPopup::SetImage(Texture* texture)
{
    // Enable image and set texture.
    mImage->SetEnabled(true);
    mImage->SetTexture(texture, true);

    // The window now needs to be big enough to show the image and any text.
    // So increase width based on image (plus margins of 10 on each side).
    // Increase height based on existing window size plus what's added by the image.
    //HACK: Adding 0.5f to avoid blurry text - a better fix would be ensuring RectTransform X/Y positions are rounded to nearest int?
    Vector2 windowSize = mWindow->GetComponent<RectTransform>()->GetSizeDelta();
    Vector2 imageSize = mImage->GetRectTransform()->GetSizeDelta();
    mWindow->GetComponent<RectTransform>()->SetSizeDelta(imageSize.x + 20, windowSize.y + imageSize.y + 0.5f);

    mMessage->GetRectTransform()->SetAnchor(AnchorPreset::TopStretch);
    mMessage->GetRectTransform()->SetAnchoredPosition(0.0f, -imageSize.y - 15);
    mMessage->GetRectTransform()->SetSizeDelta(0.0f, -15.0f);
}

void SidneyPopup::ShowOneButton()
{
    SetActive(true);

    mOKButton->SetActive(true);
    mYesButton->SetActive(false);
    mNoButton->SetActive(false);
}

void SidneyPopup::ShowTwoButton(const std::function<void()>& yesCallback)
{
    SetActive(true);

    mOKButton->SetActive(false);
    mYesButton->SetActive(true);
    mNoButton->SetActive(true);

    mYesButton->SetPressCallback([this, yesCallback](){
        SetActive(false);
        yesCallback();
    });
}
