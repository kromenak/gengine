#include "ConfirmPopup.h"

#include "AssetManager.h"
#include "IniParser.h"
#include "InputManager.h"
#include "TextAsset.h"
#include "UIButton.h"
#include "UILabel.h"
#include "UINineSlice.h"
#include "UIUtil.h"

ConfirmPopup::ConfirmPopup() : Actor("ConfirmPopup", TransformType::RectTransform),
    mLayer("ModalMessageLayer")
{
    // When the quit popup shows, all underlying audio stops entirely.
    mLayer.OverrideAudioState(true);

    // Add a fullscreen canvas that tints whatever's under it.
    UI::AddCanvas(this, 40, Color32(0, 0, 0, 128));

    // This popup reads its config from an INI text file.
    // Though there's only one that's ever used, I'll read it in, in case anyone ever wants to mod this thing.
    Font* font = nullptr;
    Texture* yesButtonUpTexture = nullptr;
    Texture* yesButtonDownTexture = nullptr;
    Texture* yesButtonHoverTexture = nullptr;
    Texture* noButtonUpTexture = nullptr;
    Texture* noButtonDownTexture = nullptr;
    Texture* noButtonHoverTexture = nullptr;
    UINineSliceParams boxParams;
    Vector2 textOffset;
    Vector2 minSize;
    {
        TextAsset* textFile = gAssetManager.LoadText("MSGBOX.TXT", AssetScope::Manual);

        // Pass that along to INI parser, since it is plain text and in INI format.
        IniParser parser(textFile->GetText(), textFile->GetTextLength());
        parser.SetMultipleKeyValuePairsPerLine(false); // Stops splitting on commas.

        while(parser.ReadLine())
        {
            while(parser.ReadKeyValuePair())
            {
                const IniKeyValue& entry = parser.GetKeyValue();
                if(entry.key[0] == ';') { continue; }

                if(StringUtil::EqualsIgnoreCase(entry.key, "Font"))
                {
                    font = gAssetManager.LoadFont(entry.value);
                }
                else if(StringUtil::EqualsIgnoreCase(entry.key, "yesSpriteUp"))
                {
                    yesButtonUpTexture = gAssetManager.LoadTexture(entry.value);
                }
                else if(StringUtil::EqualsIgnoreCase(entry.key, "yesSpriteDown"))
                {
                    yesButtonDownTexture = gAssetManager.LoadTexture(entry.value);
                }
                else if(StringUtil::EqualsIgnoreCase(entry.key, "yesSpriteHov"))
                {
                    yesButtonHoverTexture = gAssetManager.LoadTexture(entry.value);
                }
                else if(StringUtil::EqualsIgnoreCase(entry.key, "noSpriteUp"))
                {
                    noButtonUpTexture = gAssetManager.LoadTexture(entry.value);
                }
                else if(StringUtil::EqualsIgnoreCase(entry.key, "noSpriteDown"))
                {
                    noButtonDownTexture = gAssetManager.LoadTexture(entry.value);
                }
                else if(StringUtil::EqualsIgnoreCase(entry.key, "noSpriteHov"))
                {
                    noButtonHoverTexture = gAssetManager.LoadTexture(entry.value);
                }
                else if(StringUtil::EqualsIgnoreCase(entry.key, "ulCornerSprite"))
                {
                    boxParams.topLeftTexture = gAssetManager.LoadTexture(entry.value);
                }
                else if(StringUtil::EqualsIgnoreCase(entry.key, "urCornerSprite"))
                {
                    boxParams.topRightTexture = gAssetManager.LoadTexture(entry.value);
                }
                else if(StringUtil::EqualsIgnoreCase(entry.key, "llCornerSprite"))
                {
                    boxParams.bottomLeftTexture = gAssetManager.LoadTexture(entry.value);
                }
                else if(StringUtil::EqualsIgnoreCase(entry.key, "lrCornerSprite"))
                {
                    boxParams.bottomRightTexture = gAssetManager.LoadTexture(entry.value);
                }
                else if(StringUtil::EqualsIgnoreCase(entry.key, "vertSprite"))
                {
                    boxParams.leftTexture = boxParams.rightTexture = gAssetManager.LoadTexture(entry.value);
                }
                else if(StringUtil::EqualsIgnoreCase(entry.key, "horizSprite"))
                {
                    boxParams.topTexture = boxParams.bottomTexture = gAssetManager.LoadTexture(entry.value);
                }
                else if(StringUtil::EqualsIgnoreCase(entry.key, "backgroundColor"))
                {
                    Vector3 colorAsVec = entry.GetValueAsVector3();
                    Color32 color(static_cast<int>(colorAsVec.x), colorAsVec.y, colorAsVec.z);
                    boxParams.centerColor = color;
                }
                else if(StringUtil::EqualsIgnoreCase(entry.key, "minSize"))
                {
                    minSize = entry.GetValueAsVector2();
                }
                else if(StringUtil::EqualsIgnoreCase(entry.key, "textOffset"))
                {
                    textOffset = entry.GetValueAsVector2();
                    mTextOffsetY = textOffset.y;
                    textOffset.y *= -1; // because the coordinate system is flipped
                }
            }
        }

        delete textFile;
    }

    // Create background box.
    UINineSlice* box = UI::CreateWidgetActor<UINineSlice>("Box", this, boxParams);
    box->GetRectTransform()->SetSizeDelta(minSize);
    mPopupTransform = box->GetRectTransform();

    // Create label.
    UILabel* label = UI::CreateWidgetActor<UILabel>("Message", box);
    label->SetFont(font);
    label->SetText("");
    label->SetHorizontalOverflow(HorizontalOverflow::Wrap);
    label->SetVerticalOverflow(VerticalOverflow::Overflow);
    label->SetVerticalAlignment(VerticalAlignment::Top);
    label->SetHorizonalAlignment(HorizontalAlignment::Left);
    label->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
    label->GetRectTransform()->SetAnchoredPosition(textOffset);
    label->GetRectTransform()->SetSizeDeltaX(minSize.x - (textOffset.x * 2.0f));
    mMessageLabel = label;

    // Create "yes" button.
    mYesButton = UI::CreateWidgetActor<UIButton>("YesButton", box);
    mYesButton->GetRectTransform()->SetAnchor(AnchorPreset::BottomLeft);
    mYesButton->GetRectTransform()->SetAnchoredPosition(43.0f, 15.0f);
    mYesButton->SetUpTexture(yesButtonUpTexture);
    mYesButton->SetHoverTexture(yesButtonHoverTexture);
    mYesButton->SetDownTexture(yesButtonDownTexture);
    mYesButton->SetPressCallback([this](UIButton* button){
        OnYesButtonPressed();
    });

    // Create "no" button.
    mNoButton = UI::CreateWidgetActor<UIButton>("NoButton", box);
    mNoButton->GetRectTransform()->SetAnchor(AnchorPreset::BottomLeft);
    mNoButton->GetRectTransform()->SetAnchoredPosition(109.0f, 15.0f);
    mNoButton->SetUpTexture(noButtonUpTexture);
    mNoButton->SetHoverTexture(noButtonHoverTexture);
    mNoButton->SetDownTexture(noButtonDownTexture);
    mNoButton->SetPressCallback([this](UIButton* button){
        OnNoButtonPressed();
    });

    // Hide by default.
    SetActive(false);
}

void ConfirmPopup::Show(const std::string& message, const std::function<void(bool)>& callback)
{
    // Assuming this has been localized before being passed in.
    mMessageLabel->SetText(message);

    // Save callback for later use.
    mCallback = callback;

    // Calculate desired height for the popup.
    // The width is fixed, and the buttons at the bottom are always ~50 pixels in height.
    // So the question is how much space the label takes up, based on its content.
    float messageHeight = mMessageLabel->GetTextHeight();
    float boxHeight = (mTextOffsetY * 2.0f) + messageHeight + 50.0f;
    mPopupTransform->SetSizeDeltaY(boxHeight);

    // Push layer onto stack.
    gLayerManager.PushLayer(&mLayer);
    SetActive(true);
}

void ConfirmPopup::Hide()
{
    // Pop layer off stack.
    gLayerManager.PopLayer(&mLayer);
    SetActive(false);
}

void ConfirmPopup::OnUpdate(float deltaTime)
{
    // Keyboard shortcuts for yes/no buttons.
    // NOTE: original game actually doesn't have keyboard shortcuts on this popup. But feels like a good thing to have?
    if(gInputManager.IsKeyLeadingEdge(SDL_SCANCODE_RETURN))
    {
        mYesButton->AnimatePress();
    }
    else if(gInputManager.IsKeyLeadingEdge(SDL_SCANCODE_ESCAPE))
    {
        mNoButton->AnimatePress();
    }
}

void ConfirmPopup::OnYesButtonPressed()
{
    Hide();
    if(mCallback != nullptr)
    {
        mCallback(true);
    }
}

void ConfirmPopup::OnNoButtonPressed()
{
    Hide();
    if(mCallback != nullptr)
    {
        mCallback(false);
    }
}
