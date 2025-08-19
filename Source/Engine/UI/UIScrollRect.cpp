#include "UIScrollRect.h"

#include "AssetManager.h"
#include "Texture.h"
#include "UIButton.h"
#include "UIImage.h"
#include "UINineSlice.h"
#include "UISlider.h"
#include "UIUtil.h"

UIScrollRect::UIScrollRect(Actor* parent) : Actor("ScrollRect", TransformType::RectTransform)
{
    GetTransform()->SetParent(parent->GetTransform());

    // NOTE: Assuming only vertical scrolling for now...

    // Some size calculations depend on the width/height of the up/down button textures (the ones that appear on the ends of the scrollbar).
    // So, get those textures now.
    Texture* upTexture = gAssetManager.LoadTexture("S_SLIDER_UPARROW.BMP");
    Texture* downTexture = gAssetManager.LoadTexture("S_SLIDER_DWNARROW.BMP");

    // Add a child actor that contains all scrollbar widgets.
    // This is useful so that we can easily exclude the scrollbar widgets from content size calculations.
    mScrollbarActor = new Actor("Scrollbar", TransformType::RectTransform);
    mScrollbarActor->GetTransform()->SetParent(GetTransform());
    mScrollbarActor->GetComponent<RectTransform>()->SetAnchor(AnchorPreset::RightStretch);
    mScrollbarActor->GetComponent<RectTransform>()->SetAnchoredPosition(0.0f, 0.0f);
    mScrollbarActor->GetComponent<RectTransform>()->SetSizeDelta(static_cast<float>(upTexture->GetWidth()), 0.0f);

    // We need to leave some space at the top/bottom of the scroll bar for the up/down buttons. Calculate that space needed now.
    float scrollbarEndMargin = static_cast<float>(upTexture->GetHeight() + downTexture->GetHeight());

    // Create backing behind scroll area.
    // Size it such that it takes up all the scrollbar area except space at top/bottom for the two buttons.
    UIImage* backing = UI::CreateWidgetActor<UIImage>("Backing", mScrollbarActor);
    backing->SetTexture(&Texture::Black);
    backing->GetRectTransform()->SetAnchor(AnchorPreset::CenterStretch);
    backing->GetRectTransform()->SetAnchoredPosition(0.0f, 0.0f);
    backing->GetRectTransform()->SetSizeDelta(0.0f, -scrollbarEndMargin);
    mHandleBacking = backing->GetRectTransform();

    // Create up button.
    {
        UIButton* upButton = UI::CreateWidgetActor<UIButton>("UpButton", mScrollbarActor);
        upButton->SetUpTexture(upTexture);
        upButton->SetPressCallback([this](UIButton* button){
            OnUpButtonPressed();
        });
        upButton->GetRectTransform()->SetAnchor(AnchorPreset::Top);
    }

    // Create down button.
    {
        UIButton* downButton = UI::CreateWidgetActor<UIButton>("DownButton", mScrollbarActor);
        downButton->SetUpTexture(downTexture);
        downButton->SetPressCallback([this](UIButton* button){
            OnDownButtonPressed();
        });
        downButton->GetRectTransform()->SetAnchor(AnchorPreset::Bottom);
    }

    // Create slider, which you grab to scroll the scroll area.
    {
        mSlider = backing->GetOwner()->AddComponent<UISlider>();
        mSlider->SetOrientation(SliderOrientation::TopDown);

        mSlider->SetValue(0.0f);
        mSlider->SetValueChangeCallback([this](float value){
            OnSliderValueChanged(value);
        });
    }

    // The slider area is pretty useless without a handle to drag!
    {
        UINineSliceParams params;
        params.centerColor = Color32(132, 130, 132);
        params.leftColor = params.topColor = params.topLeftColor = Color32(181, 125, 0);
        params.rightColor = params.bottomColor = params.bottomLeftColor = params.bottomRightColor = params.topRightColor = Color32(90, 28, 33);

        UINineSlice* handle = UI::CreateWidgetActor<UINineSlice>("Handle", backing, params);
        handle->GetRectTransform()->SetAnchor(AnchorPreset::TopRight);
        handle->GetRectTransform()->SetAnchoredPosition(0.0f, 0.0f);
        handle->GetRectTransform()->SetSizeDelta(backing->GetRectTransform()->GetRect().width, 25.0f);
        mHandle = handle->GetRectTransform();

        mSlider->SetHandleActor(handle->GetOwner());
    }
}

void UIScrollRect::SetScrollbarWidth(float width)
{
    //HACK: Not entirely sure why - maybe due to floating point imprecision or anchor calculations or not being pixel perfect -
    //HACK: but an odd width can cause some odd jittery behavior when the UIDrag tries to keep itself within the scroll area.
    //HACK: Adding one in that case seems to help.
    if(!Math::IsZero(Math::Mod(width, 2)))
    {
        width += 1.0f;
    }

    // Change size of scrollbar and the handle.
    mScrollbarActor->GetComponent<RectTransform>()->SetSizeDeltaX(width);
    mHandle->SetSizeDeltaX(width);
}

float UIScrollRect::GetScrollbarWidth() const
{
    return mScrollbarActor->GetComponent<RectTransform>()->GetSizeDelta().x;
}

void UIScrollRect::SetNormalizedScrollValue(float normalizedScrollValue)
{
    // Slider only calls the "value changed" callback if the value actually changes.
    // If slider is set to its existing value, we should still fire the value change callback, to ensure scroll rect contents are updated.
    if(!Math::AreEqual(mSlider->GetValue(), normalizedScrollValue))
    {
        mSlider->SetValue(normalizedScrollValue);
    }
    else
    {
        OnSliderValueChanged(normalizedScrollValue);
    }
}

void UIScrollRect::OnUpdate(float deltaTime)
{
    // The scroll rect has a certain height (e.g. the viewable area of this scroll rect).
    float scrollRectHeight = GetScrollRectHeight();

    // And the children that make up the contents of the scroll rect have a height...
    float contentHeight = GetContentHeight();

    // If the contents are smaller than the scroll rect, we don't need to scroll.
    if(contentHeight <= scrollRectHeight)
    {
        mScrollDisabled = true;
        mHandle->GetOwner()->SetActive(false);
        mSlider->SetValue(0.0f);
    }
    else
    {
        // If bigger, we DO need to scroll.
        mScrollDisabled = false;
        mHandle->GetOwner()->SetActive(true);

        // The size of the handle decreases when there is more content.
        // For example, if content was twice as large as the scroll area, the handle should take up 50% of the space.
        // If the content is 4x as large as scroll area, the handle should take up 25% of the space.
        float ratio = scrollRectHeight / contentHeight;
        mHandle->SetSizeDeltaY(mHandleBacking->GetRect().height * ratio);
    }
}

float UIScrollRect::GetScrollRectHeight() const
{
    // The scroll rect's height is just our rect's height.
    return GetComponent<RectTransform>()->GetRect().height;
}

float UIScrollRect::GetContentHeight() const
{
    // You might think we could just do: GetComponent<RectTransform>()->GetWorldRect(true).height
    // Unfortunately, this won't give the correct result. As you scroll down, any empty space at the TOP of the scroll area stops being included in the calculation.

    // To get a better result, we can use this heuristic:
    // 1) Get the (0, 0) point in our local space and convert to world space.
    // 2) Find the lowest content point in our local space and convert to world space.
    // 3) Take the diff to get a consistent content height.

    // Get the scroll area world rect and add any offset we currently have.
    // The effect is that the top-left corner corresponds to (0, 0) within our local space, even if we have scrolled.
    Rect worldRect = GetComponent<RectTransform>()->GetWorldRect();
    worldRect.y += mOffset.y;
    //Debug::DrawScreenRect(worldRect, Color32::Green);

    // The top left corner represents the START of the content.
    float topLeftCorner = worldRect.GetMax().y;

    // Next, iterate children and find the LOWEST down rect's bottom left corner.
    // The bottom left corner represents the END of the content.
    float bottomLeftCorner = topLeftCorner;
    //Rect bottomWorldRect;
    for(Transform* child : GetTransform()->GetChildren())
    {
        if(child->GetOwner() == mScrollbarActor) { continue; }
        if(!child->IsActiveAndEnabled()) { continue; }

        RectTransform* rt = static_cast<RectTransform*>(child);
        Rect childWorldRect = rt->GetWorldRect(true);
        if(childWorldRect.GetMin().y < bottomLeftCorner)
        {
            bottomLeftCorner = childWorldRect.GetMin().y;
            //bottomWorldRect = childWorldRect;
        }
    }
    //Debug::DrawScreenRect(bottomWorldRect, Color32::Cyan);

    // The height is then the diff of those two.
    return Math::Abs(topLeftCorner - bottomLeftCorner);
}

void UIScrollRect::OnUpButtonPressed()
{
    if(!mScrollDisabled)
    {
        // Each press of this button moves the scroll up by X units.
        // But we need to translate that into a scroll bar value delta instead of units!
        float desiredYOffset = mOffset.y - mButtonScrollIncrement;
        float scrollAreaHeight = GetContentHeight() - GetScrollRectHeight();
        float desiredValue = desiredYOffset / scrollAreaHeight;
        mSlider->SetValue(desiredValue);
    }
}

void UIScrollRect::OnDownButtonPressed()
{
    if(!mScrollDisabled)
    {
        // Same as up button, but going in opposite direction.
        float desiredYOffset = mOffset.y + mButtonScrollIncrement;
        float scrollAreaHeight = GetContentHeight() - GetScrollRectHeight();
        float desiredValue = desiredYOffset / scrollAreaHeight;
        mSlider->SetValue(desiredValue);
    }
}

void UIScrollRect::OnSliderValueChanged(float value)
{
    // SO THIS is what ultimately controls ALL the scroll behavior. Changing the slider value implies scrolling to the corresponding position.
    // I'm not a HUGE fan of the logic here, but it seems to work...for now.

    // Get content height FIRST (before messing with transforms) to ensure accurate values.
    // If you do this between the for-loops, you get incorrect results.
    float contentHeight = GetContentHeight();
    float scrollRectHeight = GetScrollRectHeight();

    // One annoying problem: we want the user of this class to just be able to add children to it and have them scroll.
    // That means there's no parent container we can scroll up and down here - we need to scroll the children individually.
    // But that means we need to remember each child's original position somewhere...but where?

    // Solution: Instead of remembering original positions, we'll simply UNAPPLY the current offset first, putting things back in their original positions...
    for(Transform* child : GetTransform()->GetChildren())
    {
        if(child->GetOwner() == mScrollbarActor) { continue; }

        RectTransform* rt = static_cast<RectTransform*>(child);
        Vector2 anchoredPosition = rt->GetAnchoredPosition();
        anchoredPosition -= mOffset;
        rt->SetAnchoredPosition(anchoredPosition);
    }

    // But also, if the content is smaller than the scroll rect, no scrolling is needed - just set offset to zero.
    if(contentHeight <= scrollRectHeight)
    {
        mOffset.y = 0.0f;
    }
    else // content is bigger than scroll area
    {
        // Then we calculate a new offset based on the slider value.
        mOffset.y = value * (contentHeight - scrollRectHeight);

        // And then we APPLY the NEW offset to all the child items.
        for(Transform* child : GetTransform()->GetChildren())
        {
            if(child->GetOwner() == mScrollbarActor) { continue; }

            RectTransform* rt = static_cast<RectTransform*>(child);
            Vector2 anchoredPosition = rt->GetAnchoredPosition();
            anchoredPosition += mOffset;
            rt->SetAnchoredPosition(anchoredPosition);
        }
    }
}
