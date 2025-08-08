#include "UIDrag.h"

#include "CursorManager.h"
#include "InputManager.h"

TYPEINFO_INIT(UIDrag, UIWidget, 18)
{

}

UIDrag::UIDrag(Actor* owner) : UIWidget(owner)
{
    SetReceivesInput(true);
}

void UIDrag::Render()
{
    // Anything?
}

void UIDrag::OnPointerEnter()
{
    mPointerHovering = true;
    UpdateCursor();
}

void UIDrag::OnPointerExit()
{
    mPointerHovering = false;
    UpdateCursor();
}

void UIDrag::OnPointerDown()
{
    mDragging = true;
    UpdateCursor();
}

void UIDrag::OnPointerUp()
{
    mDragging = false;
    UpdateCursor();
}

void UIDrag::OnUpdate(float deltaTime)
{
    // If dragging, update transform to follow pointer.
    if(mDragging)
    {
        Vector2 mouseDelta = gInputManager.GetMouseDelta();
        if(mouseDelta.GetLengthSq() > 0.0f)
        {
            // If a drag direction restriction is present, limit the mouse delta to only the parts in that direction.
            // We use a vector operation known as "scalar projection" here (dot product of a unit vector with a non-unit vector) to achieve this.
            // The effect of scalar projection is to isolate the magnitude of the non-unit vector in the direction of the unit vector.
            if(mAllowedDragDirection.GetLengthSq() > 0)
            {
                float distInDragDir = Vector2::Dot(mouseDelta, mAllowedDragDirection);
                mouseDelta = mAllowedDragDirection * distInDragDir;
            }

            // To account for scaled parent transforms, we need to convert delta from world space to local space.
            Vector3 localMouseDelta = GetRectTransform()->GetWorldToLocalMatrix().TransformVector(mouseDelta);

            // Move anchored position to match mouse change.
            Vector2 anchoredPos = GetRectTransform()->GetAnchoredPosition();
            anchoredPos += localMouseDelta;
            GetRectTransform()->SetAnchoredPosition(anchoredPos);

            // Keep within boundary rect, if rect is valid/set.
            if(mBoundaryRectTransform != nullptr)
            {
                GetRectTransform()->MoveInsideRect(mBoundaryRectTransform->GetWorldRect());
            }
        }

        // Failsafe: in some edge cases, you can move the mouse in such a way (while releasing the mouse button) to avoid the OnPointerUp callback.
        // To combat this, if dragging, check for mouse up to cancel drag.
        if(!gInputManager.IsMouseButtonPressed(InputManager::MouseButton::Left))
        {
            mDragging = false;
            UpdateCursor();
        }
    }
}

void UIDrag::UpdateCursor()
{
    if(mUseHighlightCursor && (mDragging || mPointerHovering))
    {
        gCursorManager.UseHighlightCursor();
    }
    else
    {
        gCursorManager.UseDefaultCursor();
    }
}
