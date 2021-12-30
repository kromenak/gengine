#include "UIDrag.h"

#include "Debug.h"
#include "Services.h"

TYPE_DEF_CHILD(UIWidget, UIDrag);

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
    /*
    if(mBoundaryRect.GetSize().GetLengthSq() > 0.0f)
    {
        Debug::DrawScreenRect(mBoundaryRect, Color32::Magenta);
    }
    */

    // If dragging, update transform to follow pointer.
    if(mDragging)
    {
        Vector2 mouseDelta = Services::GetInput()->GetMouseDelta();
        if(mouseDelta.GetLengthSq() > 0.0f)
        {
            // Move anchored position to match.
            Vector2 anchoredPos = GetRectTransform()->GetAnchoredPosition();
            anchoredPos += Services::GetInput()->GetMouseDelta();
            GetRectTransform()->SetAnchoredPosition(anchoredPos);

            // Keep within boundary rect, if rect is valid/set.
            if(mBoundaryRect.GetSize().GetLengthSq() > 0.0f)
            {
                GetRectTransform()->MoveInsideRect(mBoundaryRect);
            }
        }

        // Failsafe: in some edge cases, you can move the mouse in such a way (while releasing the mouse button) to avoid the OnPointerUp callback.
        // To combat this, if dragging, check for mouse up to cancel drag.
        if(!Services::GetInput()->IsMouseButtonPressed(InputManager::MouseButton::Left))
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
        if(Services::Get<CursorManager>()->IsDefaultCursor())
        {
            Services::Get<CursorManager>()->UseHighlightCursor();
        }
    }
    else
    {
        Services::Get<CursorManager>()->UseDefaultCursor();
    }
}
