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
                // If the boundary rect is the direct parent of the UIDrag, we can clamp anchored position in local space to keep within boundary.
                // TODO: This code assumes the pivot corresponds to the anchor, which is usually the case.
                // TODO: This code assumes a singular anchor - no stretching. AnchorMin == AnchorMax.
                if(GetRectTransform()->GetParent() == mBoundaryRectTransform &&
                   GetRectTransform()->GetPivot() == GetRectTransform()->GetAnchorMin() &&
                   GetRectTransform()->GetAnchorMin() == GetRectTransform()->GetAnchorMax())
                {
                    // If anchor of UIDrag is bottom-left, MinX/MinY would be zero. If top-right, MinX/MinY are (-sizeDiff.x, -sizeDiff.y).
                    // So multiplying by the pivot (assuming pivot corresponds to anchor position) gives correct min values.
                    Vector2 boundaryRectSize = mBoundaryRectTransform->GetSize();
                    Vector2 handleSize = GetRectTransform()->GetSize();
                    float minX = (boundaryRectSize.x - handleSize.x) * -GetRectTransform()->GetPivot().x;
                    float minY = (boundaryRectSize.y - handleSize.y) * -GetRectTransform()->GetPivot().y;

                    // If anchor is bottom-left, MaxX/MaxY are just the size diff. If top-right, MaxX/MaxY are zero.
                    // So multiplying by (1-pivot) (assuming pivot corresponds to anchor position) gives correct max value.
                    float maxX = (boundaryRectSize.x - handleSize.x) * (1.0f - GetRectTransform()->GetPivot().x);
                    float maxY = (boundaryRectSize.y - handleSize.y) * (1.0f - GetRectTransform()->GetPivot().y);

                    // And then we can clamp anchored position and use that.
                    anchoredPos.x = Math::Clamp(anchoredPos.x, minX, maxX);
                    anchoredPos.y = Math::Clamp(anchoredPos.y, minY, maxY);
                    GetRectTransform()->SetAnchoredPosition(anchoredPos);
                }
                else
                {
                    // The clamping approach is nice because it does everything in a single coordinate space and is very accurate.
                    // But if it can't be achieved, we can do everything in world space.
                    // This introduces some floating point inaccuracies that can cause jitter, but it has to do worst case.
                    GetRectTransform()->MoveInsideRect(mBoundaryRectTransform->GetWorldRect());
                }
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

void UIDrag::UpdateCursor() const
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
