//
// Clark Kromenaker
//
// Base class for any UI element, called a "widget".
//
#pragma once
#include "Component.h"

#include "RectTransform.h"

enum class UIWidgetInputMode
{
    ReceivesNoInput,            // Widget can't receive any input.
    ReceivesInputWithinRect,    // Widget receives input if pointer is over the widget's rect. Most common/normal approach.
    ReceivesAllInput            // Widget ALWAYS receives input. Used mainly for fullscreen input capture.
};

class UIWidget : public Component
{
    TYPEINFO_SUB(UIWidget, Component);
public:
    UIWidget(Actor* actor);
    virtual ~UIWidget();

    virtual void Render() { }

    // Called when pointer enters/exits the bounds of this widget.
    virtual void OnPointerEnter() { }
    virtual void OnPointerExit() { }

    // Called when pointer down/up when within the bounds of this widget.
    virtual void OnPointerDown() { }
    virtual void OnPointerUp() { }

    RectTransform* GetRectTransform() const { return mRectTransform; }

    void SetReceivesInput(bool receivesInput) { mInputMode = receivesInput ? UIWidgetInputMode::ReceivesInputWithinRect : UIWidgetInputMode::ReceivesNoInput; }
    bool ReceivesInput() const { return mInputMode != UIWidgetInputMode::ReceivesNoInput; }

    void SetInputMode(UIWidgetInputMode inputMode) { mInputMode = inputMode; }
    UIWidgetInputMode GetInputMode() const { return mInputMode; }

    virtual void SetDirty() { }

protected:
    Matrix4 GetWorldTransformWithSizeForRendering();

private:
    // UI widgets usually rely heavily on a RectTransform.
    // So, we cache it here.
    RectTransform* mRectTransform = nullptr;

    // If a widget receives input, its "OnPointer" functions are called
    // AND it can block interaction with the scene (since it can "eat" inputs).
    UIWidgetInputMode mInputMode = UIWidgetInputMode::ReceivesNoInput;
};
