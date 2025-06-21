#include "SidneyPopup.h"

#include "AssetManager.h"
#include "Font.h"
#include "InputManager.h"
#include "RectTransform.h"
#include "SidneyButton.h"
#include "SidneyUtil.h"
#include "Texture.h"
#include "UIButton.h"
#include "UIImage.h"
#include "UINineSlice.h"
#include "UITextInput.h"
#include "UIUtil.h"

SidneyPopup::SidneyPopup(Actor* parent) : Actor("Sidney Popup", TransformType::RectTransform)
{
    // Create popup root, which covers whole screen and acts as an input blocker while the message is up.
    {
        // Set as child of parent object passed in.
        GetTransform()->SetParent(parent->GetTransform());

        // Add a UICanvas, so we can force this to display above other things.
        // The weird color here is because the popup should block interaction with things below it, but there's no visible dimming that occurs.
        UI::AddCanvas(this, 4, Color32(0, 0, 0, 1));
    }

    // Add the window itself.
    {
        // Add a border around the edge.
        UINineSlice* border = UI::CreateWidgetActor<UINineSlice>("Window", this, SidneyUtil::GetGrayBoxParams(Color32::Black));
        mWindow = border->GetOwner();

        // Default to centered on-screen at a certain size.
        border->GetRectTransform()->SetSizeDelta(250.0f, 172.0f);
        border->GetRectTransform()->SetAnchor(AnchorPreset::Center);
    }

    // Add message label.
    {
        mMessage = UI::CreateWidgetActor<UILabel>("Message", mWindow);
        mMessage->SetFont(gAssetManager.LoadFont("SID_TEXT_14.FON"));
        mMessage->SetHorizonalAlignment(HorizontalAlignment::Left);
        mMessage->SetHorizontalOverflow(HorizontalOverflow::Wrap);
        mMessage->SetVerticalAlignment(VerticalAlignment::Top);

        mMessage->GetRectTransform()->SetAnchor(AnchorPreset::CenterStretch);
        mMessage->GetRectTransform()->SetSizeDelta(-15.0f, -15.0f);
    }

    // Add image element.
    {
        mImage = UI::CreateWidgetActor<UIImage>("Image", mWindow);
        mImage->SetTexture(&Texture::White, true);

        mImage->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
        mImage->GetRectTransform()->SetAnchoredPosition(10.0f, -10.0f);
    }

    // Add a text input field.
    {
        mTextInput = UI::CreateWidgetActor<UITextInput>("TextInput", mWindow);
        mTextInput->SetFont(gAssetManager.LoadFont("SID_TEXT_14.FON"));
        mTextInput->AllowInputToChangeFocus(false); // Can't click elsewhere to unfocus this input.
        mTextInput->SetMaxLength(8); // Don't allow too many characters to be added.

        // Create text input field caret.
        UIImage* caretImage = UI::CreateWidgetActor<UIImage>("Caret", mTextInput);
        caretImage->SetTexture(&Texture::White);
        caretImage->SetColor(Color32(198, 170, 41));

        caretImage->GetRectTransform()->SetAnchor(AnchorPreset::LeftStretch, false);
        caretImage->GetRectTransform()->SetPivot(0.0f, 0.0f);
        caretImage->GetRectTransform()->SetSizeDelta(2.0f, 0.0f);
        caretImage->GetRectTransform()->SetAnchoredPosition(0.0f, 0.0f);

        mTextInput->SetCaret(caretImage);
        mTextInput->SetCaretBlinkInterval(0.5f);
    }

    // Add OK button.
    {
        mOKButton = new SidneyButton("OKButton", mWindow);
        mOKButton->SetFont(gAssetManager.LoadFont("SID_PDN_10_L.FON"));
        mOKButton->SetText(SidneyUtil::GetAnalyzeLocalizer().GetText("OKButton"));
        mOKButton->SetWidth(80.0f);
        mOKButton->SetHeight(13.0f);

        mOKButton->GetRectTransform()->SetAnchor(AnchorPreset::Bottom);
        mOKButton->GetRectTransform()->SetAnchoredPosition(0.0f, 8.0f);

        // Hide on button press.
        mOKButton->SetPressCallback([this](){
            Hide();
        });
    }

    // Add YES button.
    {
        mYesButton = new SidneyButton("YesButton", mWindow);
        mYesButton->SetFont(gAssetManager.LoadFont("SID_PDN_10_L.FON"));
        mYesButton->SetText(SidneyUtil::GetAnalyzeLocalizer().GetText("YesButton"));
        mYesButton->SetWidth(80.0f);
        mYesButton->SetHeight(13.0f);

        mYesButton->GetRectTransform()->SetAnchor(AnchorPreset::Bottom);
        mYesButton->GetRectTransform()->SetAnchoredPosition(-48.0f, 8.0f);

        // Hide on button press.
        mYesButton->SetPressCallback([this](){
            Hide();
        });
    }

    // Add NO button.
    {
        mNoButton = new SidneyButton("NoButton", mWindow);
        mNoButton->SetFont(gAssetManager.LoadFont("SID_PDN_10_L.FON"));
        mNoButton->SetText(SidneyUtil::GetAnalyzeLocalizer().GetText("NoButton"));
        mNoButton->SetWidth(80.0f);
        mNoButton->SetHeight(13.0f);

        mNoButton->GetRectTransform()->SetAnchor(AnchorPreset::Bottom);
        mNoButton->GetRectTransform()->SetAnchoredPosition(48.0f, 8.0f);

        // Hide on button press.
        mNoButton->SetPressCallback([this](){
            Hide();
        });
    }

    // Add buttons for three button layout.
    {
        {
            mLeftButton = new SidneyButton("LeftButton", mWindow);
            mLeftButton->SetFont(gAssetManager.LoadFont("SID_PDN_10_L.FON"));
            mLeftButton->SetText(SidneyUtil::GetAnalyzeLocalizer().GetText("French"));
            mLeftButton->SetWidth(60.0f);
            mLeftButton->SetHeight(13.0f);

            mLeftButton->GetRectTransform()->SetAnchor(AnchorPreset::Bottom);
            mLeftButton->GetRectTransform()->SetAnchoredPosition(-70.0f, 8.0f);
        }
        {
            mCenterButton = new SidneyButton("CenterButton", mWindow);
            mCenterButton->SetFont(gAssetManager.LoadFont("SID_PDN_10_L.FON"));
            mCenterButton->SetText(SidneyUtil::GetAnalyzeLocalizer().GetText("English"));
            mCenterButton->SetWidth(60.0f);
            mCenterButton->SetHeight(13.0f);

            mCenterButton->GetRectTransform()->SetAnchor(AnchorPreset::Bottom);
            mCenterButton->GetRectTransform()->SetAnchoredPosition(0.0f, 8.0f);
        }
        {
            mRightButton = new SidneyButton("RightButton", mWindow);
            mRightButton->SetFont(gAssetManager.LoadFont("SID_PDN_10_L.FON"));
            mRightButton->SetText(SidneyUtil::GetAnalyzeLocalizer().GetText("Latin"));
            mRightButton->SetWidth(60.0f);
            mRightButton->SetHeight(13.0f);

            mRightButton->GetRectTransform()->SetAnchor(AnchorPreset::Bottom);
            mRightButton->GetRectTransform()->SetAnchoredPosition(70.0f, 8.0f);
        }
    }

    // Hide by default.
    Hide();
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

    // Assume text input will be hidden by default.
    mTextInput->Clear();
    mTextInput->SetEnabled(false);
    mTextInputSubmitCallback = nullptr;
}

void SidneyPopup::SetWindowPosition(const Vector2& position)
{
    mWindow->GetComponent<RectTransform>()->SetAnchoredPosition(position);
}

void SidneyPopup::SetWindowSize(const Vector2& size)
{
    mWindow->GetComponent<RectTransform>()->SetSizeDelta(size);
}

void SidneyPopup::SetText(const std::string& message)
{
    mMessage->SetText(message);

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

void SidneyPopup::ShowTextInput(const std::function<void(const std::string&)>& submitCallback)
{
    SetActive(true);

    // Hide all buttons.
    mOKButton->SetActive(false);
    mYesButton->SetActive(false);
    mNoButton->SetActive(false);
    mLeftButton->SetActive(false);
    mCenterButton->SetActive(false);
    mRightButton->SetActive(false);

    // Save callback for later use.
    mTextInputSubmitCallback = submitCallback;

    // At the moment, the text input popup is highly specialized for showing one line of text next to an input field in the center of the popup.
    // First, calculate center of window and line height.
    Vector2 windowSize = mWindow->GetComponent<RectTransform>()->GetSizeDelta();
    float centerY = windowSize.y / 2.0f;
    float lineHeight = mMessage->GetFont()->GetGlyphHeight();

    // Position message centered vertically, to the left.
    mMessage->GetRectTransform()->SetAnchor(0.0f, 0.0f);
    mMessage->GetRectTransform()->SetPivot(0.0f, 0.5f);
    mMessage->GetRectTransform()->SetAnchoredPosition(15.0f, centerY);
    mMessage->GetRectTransform()->SetSizeDelta(windowSize.x / 2.0f, lineHeight);
    float messageWidth = mMessage->GetTextWidth();

    // Position text input next to message, on the right.
    mTextInput->SetEnabled(true);
    mTextInput->GetRectTransform()->SetAnchor(0.0f, 0.0f);
    mTextInput->GetRectTransform()->SetPivot(0.0f, 0.5f);
    mTextInput->GetRectTransform()->SetAnchoredPosition(15.0f + messageWidth + 5.0f, centerY);
    mTextInput->GetRectTransform()->SetSizeDelta(windowSize.x - 20.0f - 15.0f - messageWidth, lineHeight);

    // Focus the input.
    mTextInput->Focus();
}

void SidneyPopup::Hide()
{
    SetActive(false);
    mTextInput->Unfocus();
}

void SidneyPopup::ShowNoButton()
{
    SetActive(true);

    mOKButton->SetActive(false);
    mYesButton->SetActive(false);
    mNoButton->SetActive(false);
    mLeftButton->SetActive(false);
    mCenterButton->SetActive(false);
    mRightButton->SetActive(false);
}

void SidneyPopup::ShowOneButton(const std::function<void()>& callback)
{
    SetActive(true);

    // Show only OK button.
    mOKButton->SetActive(true);
    mYesButton->SetActive(false);
    mNoButton->SetActive(false);
    mLeftButton->SetActive(false);
    mCenterButton->SetActive(false);
    mRightButton->SetActive(false);

    mOKButton->SetPressCallback([this, callback](){
        Hide();
        if(callback != nullptr)
        {
            callback();
        }
    });
}

void SidneyPopup::ShowTwoButton(const std::function<void()>& yesCallback)
{
    SetActive(true);

    // Show yes and no buttons.
    mOKButton->SetActive(false);
    mYesButton->SetActive(true);
    mNoButton->SetActive(true);
    mLeftButton->SetActive(false);
    mCenterButton->SetActive(false);
    mRightButton->SetActive(false);

    // Set callback when yes button is pressed.
    mYesButton->SetPressCallback([this, yesCallback](){
        Hide();
        yesCallback();
    });
}

void SidneyPopup::ShowThreeButton(const std::function<void(int)>& callback)
{
    SetActive(true);

    // Show yes and no buttons.
    mOKButton->SetActive(false);
    mYesButton->SetActive(false);
    mNoButton->SetActive(false);
    mLeftButton->SetActive(true);
    mCenterButton->SetActive(true);
    mRightButton->SetActive(true);

    // Set button press callbacks.
    // For the one use of this in the game thus far, pressing these buttons *should not* hide the popup automatically.
    mLeftButton->SetPressCallback([callback](){
        callback(0);
    });
    mCenterButton->SetPressCallback([callback](){
        callback(1);
    });
    mRightButton->SetPressCallback([callback](){
        callback(2);
    });
}

void SidneyPopup::OnUpdate(float deltaTime)
{
    // In text input mode, we need to catch RETURN being pressed to submit the text.
    // This also closes the popup.
    if(mTextInput->IsEnabled() && gInputManager.IsKeyLeadingEdge(SDL_SCANCODE_RETURN))
    {
        Hide();

        // Do callback AFTER set active to false, in case callback triggers another popup.
        mTextInputSubmitCallback(mTextInput->GetText());
    }
}