#include "SidneyButton.h"

#include "AssetManager.h"
#include "AudioManager.h"
#include "Font.h"
#include "SidneyUtil.h"
#include "UIButton.h"
#include "UIImage.h"
#include "UINineSlice.h"

SidneyButton::SidneyButton(Actor* parent) : Actor("SidneyButton", TransformType::RectTransform)
{
    // Set as a child of the passed in parent right away (so UI components get added to canvas).
    GetTransform()->SetParent(parent->GetTransform());

    // All of these buttons have a solid black background.
    UIImage* backgroundImage = AddComponent<UIImage>();
    backgroundImage->SetColor(Color32::Black);

    // Add label.
    Actor* labelActor = new Actor(TransformType::RectTransform);
    labelActor->GetTransform()->SetParent(GetTransform());
    mLabel = labelActor->AddComponent<UILabel>();
    mLabel->SetHorizonalAlignment(HorizontalAlignment::Center);
    mLabel->SetVerticalAlignment(VerticalAlignment::Center);
    mLabel->SetMasked(true);

    mLabel->GetRectTransform()->SetAnchor(AnchorPreset::CenterStretch);
    mLabel->GetRectTransform()->SetSizeDelta(-4.0f, 0.0f);

    // Add outer border box in gray.
    Actor* topActor = new Actor(TransformType::RectTransform);
    topActor->GetTransform()->SetParent(GetTransform());

    mBorder = topActor->AddComponent<UINineSlice>(SidneyUtil::GetGrayBoxParams(Color32::Clear));
    mBorder->GetRectTransform()->SetAnchor(AnchorPreset::CenterStretch);
    mBorder->GetRectTransform()->SetSizeDelta(0.0f, 0.0f);

    // Add button over the entire thing.
    mButton = topActor->AddComponent<UIButton>();
    mButton->SetPressCallback([this](UIButton* button) {
        Press();
    });

    // Use a default button press sound.
    mPressAudio = gAssetManager.LoadAudio("SIDBUTTON5.WAV");
}

void SidneyButton::OnUpdate(float deltaTime)
{
    // If no press anim is occurring, just change the border color when hovering.
    mLabel->SetFont(mButton->CanInteract() ? mFont : mDisabledFont);
    if(!mButton->CanInteract())
    {
        mBorder->SetTexturesAndColors(SidneyUtil::GetGrayBoxParams(Color32::Clear));
    }
    else if(mPressAnimTimer <= 0.0f)
    {
        if(mButton->IsHovered())
        {
            mBorder->SetTexturesAndColors(SidneyUtil::GetGoldBoxParams(Color32::Clear));
        }
        else
        {
            mBorder->SetTexturesAndColors(SidneyUtil::GetGrayBoxParams(Color32::Clear));
        }
    }
    else
    {
        // During a press animation, decrement the timer.
        mPressAnimTimer -= deltaTime;
        
        // Calculate whether the button border should be gold or gray. During this anim, it flashes back and forth quickly.
        // Since the timer is short (~0.3f), multiplying by 10.0f causes a blink about every 0.1 seconds.
        // Using values greater than 10.0f will make it blink faster and more often. 
        int blinkDiv = static_cast<int>(mPressAnimTimer * 20.0f);
        if(blinkDiv % 2 == 0)
        {
            mBorder->SetTexturesAndColors(SidneyUtil::GetGoldBoxParams(Color32::Clear));
        }
        else
        {
            mBorder->SetTexturesAndColors(SidneyUtil::GetGrayBoxParams(Color32::Clear));
        }

        // If the anim is over, execute the actual callback.
        if(mPressAnimTimer <= 0.0f)
        {
            if(mPressCallback != nullptr)
            {
                mPressCallback();
            }
        }
    }
}

void SidneyButton::SetWidth(float width)
{
    GetRectTransform()->SetSizeDeltaX(width);
}

void SidneyButton::SetFont(Font* font, Font* disabledFont)
{
    mFont = font;
    mDisabledFont = disabledFont;
    if(mDisabledFont == nullptr)
    {
        mDisabledFont = font;
    }

    // Note: assuming that a valid size is used.
    mLabel->SetFont(font);

    // Use the font's glyph height by default.
    GetRectTransform()->SetSizeDeltaY(font->GetGlyphHeight());
}

void SidneyButton::SetText(const std::string& text)
{
    mLabel->SetText(text);
}

void SidneyButton::SetTextAlignment(HorizontalAlignment alignment)
{
    mLabel->SetHorizonalAlignment(alignment);
}

RectTransform* SidneyButton::GetRectTransform()
{
    return static_cast<RectTransform*>(GetTransform());
}

void SidneyButton::Press()
{
    if(mButton->CanInteract())
    {
        gAudioManager.PlaySFX(mPressAudio);
        mPressAnimTimer = 0.3f;
    }
}