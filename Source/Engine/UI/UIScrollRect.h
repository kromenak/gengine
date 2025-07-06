//
// Clark Kromenaker
//
// A UI Actor that allows you to scroll its contents.
// Automatically detects the size of child RectTransforms and provides expected scroll behavior.
//
// Note that this DOES NOT provide masking currently. Place inside a dedicated UICanvas for that.
//
#pragma once
#include "Actor.h"

#include "RectTransform.h"

class UISlider;

class UIScrollRect : public Actor
{
public:
    UIScrollRect(Actor* parent);

    void SetScrollbarWidth(float width);
    float GetScrollbarWidth() const;

    void SetButtonScrollIncrement(float increment) { mButtonScrollIncrement = increment; }

    void SetNormalizedScrollValue(float normalizedScrollValue);

    RectTransform* GetRectTransform() const { return static_cast<RectTransform*>(GetTransform()); }

protected:
    void OnUpdate(float deltaTime) override;

private:
    // The actor containing the scrollbar widgets.
    Actor* mScrollbarActor = nullptr;

    // The area that the handle scrolls within. Needed for size calculations.
    RectTransform* mHandleBacking = nullptr;

    // Rect transform of the scroll handle. Needed to adjust size of handle based on contents.
    RectTransform* mHandle = nullptr;

    // The slider used for the scrollbar drag behavior.
    UISlider* mSlider = nullptr;

    // If true, the scroll area is disabled because there isn't enough content to scroll.
    bool mScrollDisabled = false;

    // The current offset of the content based on current scroll slider value.
    Vector2 mOffset;

    // How much we should scroll when a button (e.g. up or down) is pressed.
    float mButtonScrollIncrement = 16.0f;

    float GetScrollRectHeight() const;
    float GetContentHeight() const;

    void OnUpButtonPressed();
    void OnDownButtonPressed();
    void OnSliderValueChanged(float value);
};