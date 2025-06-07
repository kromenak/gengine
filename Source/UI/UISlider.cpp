#include "UISlider.h"

#include "Actor.h"
#include "UIDrag.h"
#include "Vector2.h"

TYPEINFO_INIT(UISlider, UIWidget, 26)
{

}

UISlider::UISlider(Actor* owner) : UIWidget(owner)
{
    SetReceivesInput(true);
}

void UISlider::SetOrientation(SliderOrientation orientation)
{
    // Save orientation.
    mOrientation = orientation;

    // Set handle anchor/pivot so that at anchored position (0, 0) it'll be at value 0.
    // Also, limit the drag direction of the handle based on orientation.
    if(mHandle != nullptr)
    {
        switch(orientation)
        {
        case SliderOrientation::LeftRight:
            mHandle->GetRectTransform()->SetAnchor(AnchorPreset::Left);
            mHandle->SetAllowedDragDirection(Vector2::UnitX);
            break;
        case SliderOrientation::RightLeft:
            mHandle->GetRectTransform()->SetAnchor(AnchorPreset::Right);
            mHandle->SetAllowedDragDirection(Vector2::UnitX);
            break;
        case SliderOrientation::BottomUp:
            mHandle->GetRectTransform()->SetAnchor(AnchorPreset::Bottom);
            mHandle->SetAllowedDragDirection(Vector2::UnitY);
            break;
        case SliderOrientation::TopDown:
            mHandle->GetRectTransform()->SetAnchor(AnchorPreset::Top);
            mHandle->SetAllowedDragDirection(Vector2::UnitY);
        }
    }

    // Make sure slider is in the correct spot based on the value.
    SetHandleFromValue();
}

void UISlider::SetHandleActor(Actor* handleActor)
{
    // We'll require that the handle is a child of the slider itself.
    handleActor->GetTransform()->SetParent(GetRectTransform());

    // Grab drag component. If one isn't on the handle, just add one now!
    mHandle = handleActor->GetComponent<UIDrag>();
    if(mHandle == nullptr)
    {
        mHandle = handleActor->AddComponent<UIDrag>();
    }

    // Make sure handle is anchored correctly based on desired orientation.
    SetOrientation(mOrientation);

    // Confine drag within the slider area.
    mHandle->SetBoundaryRectTransform(GetRectTransform());
}

void UISlider::SetValue(float value)
{
    // Save old value, to see if it changed.
    float oldValue = mValue;

    // Value must be 0 to 1.
    mValue = Math::Clamp(value, 0.0f, 1.0f);

    // Issue callback if value changed.
    if(!Math::AreEqual(mValue, oldValue) && mValueChangeCallback != nullptr)
    {
        mValueChangeCallback(mValue);
    }
}

void UISlider::SetValueSilently(float value)
{
    // Value must be 0 to 1.
    mValue = Math::Clamp(value, 0.0f, 1.0f);
}

void UISlider::OnUpdate(float deltaTime)
{
    if(mHandle != nullptr)
    {
        // If handle is being dragged, the value is driven by handle position.
        // If handle is not dragged, handle position is driven by value.
        if(mHandle->IsDragging())
        {
            SetValueFromHandle();
        }
        else
        {
            SetHandleFromValue();
        }
    }
}

void UISlider::SetHandleFromValue()
{
    if(mHandle != nullptr)
    {
        if(mOrientation == SliderOrientation::LeftRight || mOrientation == SliderOrientation::RightLeft)
        {
            // Determine width of movement area for slider.
            // This'll be width of slider, MINUS width of handle.
            float width = GetRectTransform()->GetSize().x;
            float handleWidth = mHandle->GetRectTransform()->GetSize().x;
            float actualWidth = width - handleWidth;

            // Calculate new handle pos.
            Vector2 handlePos = mHandle->GetRectTransform()->GetAnchoredPosition();
            handlePos.x = mValue * actualWidth;
            if(mOrientation == SliderOrientation::RightLeft)
            {
                handlePos.x *= -1;
            }
            mHandle->GetRectTransform()->SetAnchoredPosition(handlePos);
        }
        else
        {
            // Similar to above, but we use height and y-pos.
            float height = GetRectTransform()->GetSize().y;
            float handleHeight = mHandle->GetRectTransform()->GetSize().y;
            float actualHeight = height - handleHeight;

            // Calculate new handle pos.
            Vector2 handlePos = mHandle->GetRectTransform()->GetAnchoredPosition();
            handlePos.y = mValue * actualHeight;
            if(mOrientation == SliderOrientation::TopDown)
            {
                handlePos.y *= -1;
            }
            mHandle->GetRectTransform()->SetAnchoredPosition(handlePos);
        }
    }
}

void UISlider::SetValueFromHandle()
{
    if(mHandle != nullptr)
    {
        if(mOrientation == SliderOrientation::LeftRight || mOrientation == SliderOrientation::RightLeft)
        {
            // As above, actual move area is slider width MINUS handle width.
            float width = GetRectTransform()->GetSize().x;
            float handleWidth = mHandle->GetRectTransform()->GetSize().x;
            float actualWidth = width - handleWidth;

            // The handle pos is from the middle-left side of the handle, since we set the pivot to (0.0, 0.5).
            // Therefore, the "max" position of the handle is (width - handleWidth). We need to take this into account for the final equation.
            float handlePos = Math::Abs(mHandle->GetRectTransform()->GetAnchoredPosition().x);
            SetValue(handlePos / actualWidth);
        }
        else
        {
            // Same but with height and y-pos.
            float height = GetRectTransform()->GetSize().y;
            float handleHeight = mHandle->GetRectTransform()->GetSize().y;
            float actualHeight = height - handleHeight;

            float handlePos = Math::Abs(mHandle->GetRectTransform()->GetAnchoredPosition().y);
            SetValue(handlePos / actualHeight);
        }
    }
}