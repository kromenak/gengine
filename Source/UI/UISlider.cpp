#include "UISlider.h"

#include "Actor.h"
#include "UIDrag.h"
#include "Vector2.h"

TYPE_DEF_CHILD(UIWidget, UISlider);

UISlider::UISlider(Actor* owner) : UIWidget(owner)
{
    SetReceivesInput(true);
}

void UISlider::Render()
{
    // Anything?
}

void UISlider::SetHandleActor(Actor* handleActor)
{
    // We'll require that the handle is a child of the slider itself.
    handleActor->GetTransform()->SetParent(GetRectTransform());

    // Grab drag component.
    mHandle = handleActor->GetComponent<UIDrag>();
    if(mHandle == nullptr)
    {
        mHandle = handleActor->AddComponent<UIDrag>();
    }
    
    // We're going to assume for now: this is a horizontal slider with min value on left, max value on right.
    // Set handle anchor/pivot so that at anchored position (0, 0) it'll be at value 0.
    mHandle->GetRectTransform()->SetAnchor(0.0f, 0.5f);
    mHandle->GetRectTransform()->SetPivot(0.0f, 0.5f);
    mHandle->GetRectTransform()->SetAnchoredPosition(0.0f, 0.0f);

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

float UISlider::GetValue() const
{
    return mValue;
}

void UISlider::OnUpdate(float deltaTime)
{
    if(mHandle != nullptr)
    {
        // If handle is being dragged, the value is driven by slider position.
        // If handle is not dragged, slider position is driven by value.
        if(mHandle->IsDragging())
        {
            SetValueFromSlider();
        }
        else
        {
            SetSliderFromValue();
        }
    }
}

void UISlider::SetSliderFromValue()
{
    if(mHandle != nullptr)
    {
        // Determine width of movement area for slider.
        // This'll be width of slider, MINUS width of handle.
        float width = GetRectTransform()->GetSize().x;
        float handleWidth = mHandle->GetRectTransform()->GetSize().x;
        float actualWidth = width - handleWidth;

        // Calculate new handle pos.
        Vector2 handlePos = mHandle->GetRectTransform()->GetAnchoredPosition();
        handlePos.x = mValue * actualWidth;
        mHandle->GetRectTransform()->SetAnchoredPosition(handlePos);
    }
}

void UISlider::SetValueFromSlider()
{
    if(mHandle != nullptr)
    {
        // As above, actual move area is slider width MINUS handle width.
        float width = GetRectTransform()->GetSize().x;
        float handleWidth = mHandle->GetRectTransform()->GetSize().x;
        float actualWidth = width - handleWidth;

        // The handle pos is from the middle-left side of the handle, since we set the pivot to (0.0, 0.5).
        // Therefore, the "max" position of the handle is (width - handleWidth). We need to take this into account for the final equation.
        float handlePos = mHandle->GetRectTransform()->GetAnchoredPosition().x;
        SetValue(handlePos / actualWidth);
    }
}