#include "CursorManager.h"

#include "AssetManager.h"
#include "Cursor.h"

CursorManager gCursorManager;

void CursorManager::Init()
{
    // Can't do this in constructor b/c Assets might not yet be available.
    mDefaultCursor = gAssetManager.LoadCursor("C_POINT.CUR");
    mHighlightRedCursor = gAssetManager.LoadCursor("C_ZOOM.CUR");
    mHighlightBlueCursor = gAssetManager.LoadCursor("C_ZOOM_2.CUR");
    mWaitCursor = gAssetManager.LoadCursor("C_WAIT.CUR");
}

void CursorManager::Update(float deltaTime)
{
    // If there's a desired cursor, use it.
    if(mDesiredCursor != nullptr)
    {
        // If the desired cursor differs from the active cursor (or animation mode is different), change active cursor.
        if(mDesiredCursor != mActiveCursor || mDesiredCursorAnimate != mActiveCursor->IsAnimating())
        {
            mActiveCursor = mDesiredCursor;
            mActiveCursor->Activate(mDesiredCursorAnimate);
        }

        // Reset desired cursor tracking vars.
        mDesiredCursor = nullptr;
        mDesiredCursorPriority = -1;
        mDesiredCursorAnimate = true;
    }

    // Update active cursor.
    if(mActiveCursor != nullptr)
    {
        mActiveCursor->Update(deltaTime);
    }
}

void CursorManager::UseDefaultCursor(int priority)
{
    // The default cursor has low priority - only use it if nothing of higher priority is specified.
    SetDesiredCursor(mDefaultCursor, priority);
}

void CursorManager::UseRedHighlightCursor(int priority)
{
    SetDesiredCursor(mHighlightRedCursor, priority);
}

void CursorManager::UseHighlightCursor(int priority)
{
    // To help visualize interactable objects that are very close to one another,
    // the highlight cursor toggles from red to blue if activated when highlight is already active.
    Cursor* useCursor = mHighlightRedCursor;
    if(mHighlightRedCursor == nullptr || mActiveCursor == mHighlightRedCursor)
    {
        useCursor = mHighlightBlueCursor;
    }
    SetDesiredCursor(useCursor, priority);
}

void CursorManager::UseWaitCursor()
{
    // "Wait" cursor is just the "loading" cursor, but it doesn't animate.
    // It always has a very high priority.
    SetDesiredCursor(mWaitCursor, 100, false);
}

void CursorManager::UseLoadCursor()
{
    // Like the wait cursor, always has a high priority.
    SetDesiredCursor(mWaitCursor, 100);
}

void CursorManager::UseCustomCursor(Cursor* cursor, int priority)
{
    SetDesiredCursor(cursor, priority);
}

void CursorManager::SetDesiredCursor(Cursor* cursor, int priority, bool animate)
{
    // Switch to the passed in cursor if the associated priority is greater or equal to anything we've yet been asked to use.
    if(priority >= mDesiredCursorPriority)
    {
        mDesiredCursor = cursor;
        mDesiredCursorPriority = priority;
        mDesiredCursorAnimate = animate;
    }
}
