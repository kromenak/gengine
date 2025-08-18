#include "SidneyFakeInputPopup.h"

#include "ActionManager.h"
#include "AssetManager.h"
#include "AudioManager.h"
#include "Random.h"
#include "SidneyButton.h"
#include "SidneyUtil.h"
#include "UIButton.h"
#include "UICanvas.h"
#include "UILabel.h"
#include "UINineSlice.h"
#include "UIUtil.h"

SidneyFakeInputPopup::SidneyFakeInputPopup(Actor* parent, const std::string& name) : Actor(name, TransformType::RectTransform)
{
    // Make a child of the passed in parent.
    GetTransform()->SetParent(parent->GetTransform());

    // Add a canvas.
    AddComponent<UICanvas>(2);

    // The size is consistent in all use-cases in the game.
    // (Added one extra pixel of width so border is pixel-aligned.)
    GetComponent<RectTransform>()->SetSizeDelta(241.0f, 105.0f);

    // Add a box with border around it.
    UINineSlice* boxImage = UI::CreateWidgetActor<UINineSlice>("Box", this, SidneyUtil::GetGrayBoxParams(Color32::Black));
    boxImage->GetRectTransform()->SetAnchor(AnchorPreset::CenterStretch);

    {
        mHeaderLabel = UI::CreateWidgetActor<UILabel>("HeaderLabel", this);
        mHeaderLabel->SetFont(gAssetManager.LoadFont("SID_TEXT_14.FON"));
        mHeaderLabel->SetText("");
        mHeaderLabel->SetVerticalAlignment(VerticalAlignment::Top);

        mHeaderLabel->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
        mHeaderLabel->GetRectTransform()->SetAnchoredPosition(15.0f, -12.5f); // .5 to keep it pixel-perfect (since height of box is odd)
        mHeaderLabel->GetRectTransform()->SetSizeDelta(226.0f, 42.0f);
    }
    {
        mPromptLabel = UI::CreateWidgetActor<UILabel>("PromptLabel", this);
        mPromptLabel->SetFont(gAssetManager.LoadFont("SID_TEXT_14.FON"));
        mPromptLabel->SetText("");
        mPromptLabel->SetVerticalAlignment(VerticalAlignment::Top);

        mPromptLabel->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
        mPromptLabel->GetRectTransform()->SetAnchoredPosition(15.0f, -34.5f); // .5 to keep it pixel-perfect (since height of box is odd)
        mPromptLabel->GetRectTransform()->SetSizeDelta(226.0f, 42.0f);
    }
    {
        mInputLabel = UI::CreateWidgetActor<UILabel>("InputLabel", this);
        mInputLabel->SetFont(gAssetManager.LoadFont("SID_TEXT_14.FON"));
        mInputLabel->SetText("");
        mInputLabel->SetVerticalAlignment(VerticalAlignment::Top);

        mInputLabel->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
        mInputLabel->GetRectTransform()->SetAnchoredPosition(15.0f, -56.5f); // .5 to keep it pixel-perfect (since height of box is odd)
        mInputLabel->GetRectTransform()->SetSizeDelta(226.0f, 16.0f);

        mOKButton = new SidneyButton("OKButton", this);
        mOKButton->SetFont(gAssetManager.LoadFont("SID_PDN_10_L.FON"));
        mOKButton->SetText(SidneyUtil::GetAddDataLocalizer().GetText("OKButton"));
        mOKButton->GetButton()->SetCanInteract(false);

        mOKButton->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
        mOKButton->GetRectTransform()->SetAnchoredPosition(15.0f, -77.5f);
        mOKButton->SetWidth(40.0f);
        mOKButton->SetHeight(13.0f);
    }

    // Hide by default.
    Hide();
}

void SidneyFakeInputPopup::Show(const std::string& headerText, const std::string& promptText, const std::string& textToType, std::function<void()> doneCallback)
{
    // Set header and prompt label texts.
    mHeaderLabel->SetText(headerText);
    mPromptLabel->SetText(promptText);

    // Save the text that will be typed onto the screen.
    mTextToType = textToType;

    // Save done callback.
    mDoneCallback = doneCallback;

    // Reset type text.
    mInputLabel->SetText("");

    // Start typing at first letter after some random amount of time has passed.
    mTextToTypeIndex = 0;
    mTextToTypeTimer = Random::Range(kMinMaxTypeInterval.x, kMinMaxTypeInterval.y);

    // Activate this UI.
    SetActive(true);

    // During typing, this popup counts as an action (causes the "wait" cursor to show, game is non-interactive).
    gActionManager.StartManualAction();
}

void SidneyFakeInputPopup::Hide()
{
    SetActive(false);

    // Done typing, finish the manual action we started in Show.
    gActionManager.FinishManualAction();
}

void SidneyFakeInputPopup::OnUpdate(float deltaTime)
{
    // Wait until enough time has passed to type the next letter.
    if(mTextToTypeTimer > 0.0f)
    {
        mTextToTypeTimer -= deltaTime;
        if(mTextToTypeTimer <= 0.0f)
        {
            // We have more text to type?
            if(mTextToTypeIndex < mTextToType.size())
            {
                // Add a letter.
                mInputLabel->SetText(mTextToType.substr(0, mTextToTypeIndex + 1));
                ++mTextToTypeIndex;

                // Reroll timer for next letter.
                if(mTextToTypeIndex < mTextToType.size())
                {
                    mTextToTypeTimer = Random::Range(kMinMaxTypeInterval.x, kMinMaxTypeInterval.y);
                }
                else
                {
                    // When all letters are typed, always use a constant one-second delay before closing the box.
                    mTextToTypeTimer = 1.0f;
                }

                // Play random "key press" SFX from set of sounds.
                int index = Random::Range(1, 5);
                Audio* audio = gAssetManager.LoadAudio("COMPKEYSIN" + std::to_string(index), AssetScope::Scene);
                gAudioManager.PlaySFX(audio);
            }
            else
            {
                // We typed everything and waited a moment.
                // Programmatically press the OK button and hide the box.
                mOKButton->SetPressCallback([this]() {
                    Hide();
                    mOKButton->GetButton()->SetCanInteract(false);
                    if(mDoneCallback != nullptr)
                    {
                        mDoneCallback();
                    }
                });
                mOKButton->GetButton()->SetCanInteract(true);
                mOKButton->Press();

                // The game also plays like an "enter key press" sound at this point.
                gAudioManager.PlaySFX(gAssetManager.LoadAudio("COMPKEYSPACE.WAV", AssetScope::Scene));
            }
        }
    }
}
