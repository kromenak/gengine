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
    TYPEINFO_SUB(UIDrag, UIWidget);
public:
    UIDrag(Actor* owner);

    void Render() override;

    void OnPointerEnter() override;
    void OnPointerExit() override;

    void OnPointerDown() override;
    void OnPointerUp() override;

    void SetUseHighlightCursor(bool useHighlightCursor) { mUseHighlightCursor = useHighlightCursor; }

    void SetBoundaryRectTransform(RectTransform* rectTransform) { mBoundaryRectTransform = rectTransform; }
    void SetAllowedDragDirection(const Vector2& direction) { mAllowedDragDirection = Vector2::Normalize(direction); }

    bool IsDragging() const { return mDragging; }

protected:
    void OnUpdate(float deltaTime) override;

private:
    // If true, the widget is being actively dragged.
    bool mDragging = false;

    // Is pointer over this widget?
    bool mPointerHovering = false;

    // If set, the dragged object will stay within this rect.
    RectTransform* mBoundaryRectTransform = nullptr;

    // If non-zero, dragging will be limited to only this direction (and opposite direction).
    // For example, set this to (1, 0) to only allow horizontal dragging.
    Vector2 mAllowedDragDirection;

    // If true, use highlight cursor to indicate this is interactive.
    bool mUseHighlightCursor = true;

    void UpdateCursor();
};