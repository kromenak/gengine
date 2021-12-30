//
// Clark Kromenaker
//
// Attach to a UI component to enable clicking/dragging the thing.
//
#pragma once
#include "UIWidget.h"

#include "Rect.h"
#include "Vector2.h"

class UIDrag : public UIWidget
{
    TYPE_DECL_CHILD();
public:
    UIDrag(Actor* owner);

    void Render() override;

    void OnPointerEnter() override;
    void OnPointerExit() override;

    void OnPointerDown() override;
    void OnPointerUp() override;

    void SetBoundaryRect(const Rect& boundaryRect) { mBoundaryRect = boundaryRect; }
    void SetUseHighlightCursor(bool useHighlightCursor) { mUseHighlightCursor = useHighlightCursor; }

    bool IsDragging() const { return mDragging; }

protected:
    void OnUpdate(float deltaTime) override;

private:
    // If true, the widget is being actively dragged.
    bool mDragging = false;

    // Is pointer over this widget?
    bool mPointerHovering = false;

    // If set, the dragged object will stay within this rect.
    Rect mBoundaryRect;

    // If true, use highlight cursor to indicate this is interactive.
    bool mUseHighlightCursor = true;

    void UpdateCursor();
};