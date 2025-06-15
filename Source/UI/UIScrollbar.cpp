#include "UIScrollbar.h"

#include "Texture.h"
#include "UIButton.h"
#include "UIDrag.h"
#include "UIImage.h"
#include "UISlider.h"
#include "UIUtil.h"

UIScrollbar::UIScrollbar(Actor* owner, const UIScrollbarParams& params) : UIWidget(owner)
{
    // NOTE: Assuming only vertical scrolling for now...

    // Some size calculations depend on the width/height of the up/down button textures (the ones that appear on the ends of the scrollbar).
    // So, get those textures now.
    Texture* decreaseButtonTexture = params.decreaseValueButtonUp;
    Texture* increaseButtonTexture = params.increaseValueButtonUp;

    // Commonly, the scrollbar will be on the right-edge of its parent, and stretch to fill its parent's rect.
    // Of course, the parent can change this after creation for less common use cases.
    GetRectTransform()->SetAnchor(AnchorPreset::RightStretch);

    // Derive width of scrollbar from passed in textures.
    uint32_t scrollbarWidth = 32.0f;
    if(params.scrollbarBacking != nullptr)
    {
        scrollbarWidth = params.scrollbarBacking->GetWidth();
    }
    else if(decreaseButtonTexture != nullptr)
    {
        scrollbarWidth = decreaseButtonTexture->GetWidth();
    }
    GetRectTransform()->SetSizeDelta(static_cast<float>(scrollbarWidth), 0.0f);

    // We need to leave some space at the top/bottom of the scroll bar for the up/down buttons. Calculate that space needed now.
    int scrollbarEndMargin = 0;
    if(decreaseButtonTexture != nullptr)
    {
        scrollbarEndMargin += decreaseButtonTexture->GetHeight();
    }
    if(increaseButtonTexture != nullptr)
    {
        scrollbarEndMargin += increaseButtonTexture->GetHeight();
    }

    // Create backing behind scroll area.
    // Size it such that it takes up all the scrollbar area except space at top/bottom for the two buttons.
    UIImage* backing = UI::CreateWidgetActor<UIImage>("Backing", this);
    backing->SetTexture(params.scrollbarBacking);
    backing->SetColor(params.scrollbarBackingColor);
    backing->SetRenderMode(UIImage::RenderMode::Tiled);
    backing->GetRectTransform()->SetAnchor(AnchorPreset::CenterStretch);
    backing->GetRectTransform()->SetSizeDelta(0.0f, static_cast<float>(-scrollbarEndMargin));
    mHandleBacking = backing->GetRectTransform();

    // Create decrease value button.
    {
        mDecreaseValueButton = UI::CreateWidgetActor<UIButton>("DecScrollButton", this);
        mDecreaseValueButton->SetUpTexture(decreaseButtonTexture);
        mDecreaseValueButton->SetDownTexture(params.decreaseValueButtonDown);
        mDecreaseValueButton->SetPressCallback([this](UIButton* button){
            OnDecreaseValueButtonPressed();
        });
        mDecreaseValueButton->GetRectTransform()->SetAnchor(AnchorPreset::Top);
    }

    // Create increase value button.
    {
        mIncreaseValueButton = UI::CreateWidgetActor<UIButton>("IncScrollButton", this);
        mIncreaseValueButton->SetUpTexture(increaseButtonTexture);
        mIncreaseValueButton->SetDownTexture(params.increaseValueButtonDown);
        mIncreaseValueButton->SetPressCallback([this](UIButton* button){
            OnIncreaseValueButtonPressed();
        });
        mIncreaseValueButton->GetRectTransform()->SetAnchor(AnchorPreset::Bottom);
    }

    // Create slider, which you grab to scroll the scroll area.
    {
        mSlider = backing->GetOwner()->AddComponent<UISlider>();
        mSlider->SetOrientation(SliderOrientation::TopDown);
        mSlider->SetValueChangeCallback([this](float value){
            OnSliderValueChanged(value);
        });
    }

    // The slider area is pretty useless without a handle to drag!
    {
        // The handle is sized to match the width of the backing, with a default height (caller can change this after creation).
        UINineSlice* handle = UI::CreateWidgetActor<UINineSlice>("Handle", backing, params.handleParams);
        handle->GetRectTransform()->SetAnchor(AnchorPreset::TopRight);
        handle->GetRectTransform()->SetSizeDelta(backing->GetRectTransform()->GetRect().width, 25.0f);

        mSlider->SetHandleActor(handle->GetOwner());
        mHandle = handle->GetOwner()->GetComponent<UIDrag>();
    }
}

void UIScrollbar::SetValue(float normalizedValue)
{
    // Slider only calls the "value changed" callback if the value actually changes.
    // If value isn't changing, still fire on value changed callback, in case some layout/positioning controlled by scrollbar needs to change.
    if(!Math::AreEqual(mSlider->GetValue(), normalizedValue))
    {
        mSlider->SetValue(normalizedValue);
    }
    else
    {
        OnSliderValueChanged(normalizedValue);
    }
}

void UIScrollbar::SetValueSilently(float normalizedValue)
{
    mSlider->SetValueSilently(normalizedValue);
}

void UIScrollbar::SetHandleSize(float size)
{
    mHandle->GetRectTransform()->SetSizeDeltaY(size);
}

void UIScrollbar::SetHandleNormalizedSize(float size)
{
    // Sets size as a percent of backing height.
    mHandle->GetRectTransform()->SetSizeDeltaY(mHandleBacking->GetRect().height * size);
}

bool UIScrollbar::IsHandleBeingDragged() const
{
    return mHandle->IsDragging();
}

void UIScrollbar::SetCanInteract(bool canInteract)
{
    // Only show the handle if you can interact.
    mHandle->GetOwner()->SetActive(canInteract);

    // Only allow sliding or pressing buttons if can interact.
    mSlider->SetEnabled(canInteract);
    mDecreaseValueButton->SetCanInteract(canInteract);
    mIncreaseValueButton->SetCanInteract(canInteract);
}

void UIScrollbar::OnDecreaseValueButtonPressed()
{
    if(mHandle->GetOwner()->IsActive() && mDecreaseValueCallback != nullptr)
    {
        mDecreaseValueCallback();
    }
}

void UIScrollbar::OnIncreaseValueButtonPressed()
{
    if(mHandle->GetOwner()->IsActive() && mIncreaseValueCallback != nullptr)
    {
        mIncreaseValueCallback();
    }
}

void UIScrollbar::OnSliderValueChanged(float value)
{
    if(mValueChangeCallback != nullptr)
    {
        mValueChangeCallback(value);
    }
}