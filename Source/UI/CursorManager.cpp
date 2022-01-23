#include "CursorManager.h"

#include "ActionManager.h"
#include "Cursor.h"
#include "Loader.h"
#include "Services.h"

TYPE_DEF_BASE(CursorManager);

void CursorManager::Init()
{
    // Can't do this in constructor b/c Assets might not yet be available.
    mDefaultCursor = Services::GetAssets()->LoadCursor("C_POINT.CUR");
    mHighlightRedCursor = Services::GetAssets()->LoadCursor("C_ZOOM.CUR");
    mHighlightBlueCursor = Services::GetAssets()->LoadCursor("C_ZOOM_2.CUR");
    mWaitCursor = Services::GetAssets()->LoadCursor("C_WAIT.CUR");
}

void CursorManager::Update(float deltaTime)
{
    //TODO: Don't really like the coupling to Loader/ActionManager here.
    // If a sheep is running, show "wait" cursor. If not, go back to normal cursor.
    if(Loader::IsLoading())
    {
        UseLoadCursor();
    }
    else if(Services::Get<ActionManager>()->IsActionPlaying())
    {
        UseWaitCursor();
    }
    else
    {
        if(mActiveCursor == mWaitCursor)
        {
            UseDefaultCursor();
        }
    }

    // Update active cursor.
    if(mActiveCursor != nullptr)
    {
        mActiveCursor->Update(deltaTime);
    }
}

void CursorManager::UseDefaultCursor()
{
    if(mDefaultCursor != nullptr && mActiveCursor != mDefaultCursor)
    {
        mActiveCursor = mDefaultCursor;
        mActiveCursor->Activate();
    }
}

void CursorManager::UseHighlightCursor()
{
    // To help visualize interactable objects that are very close to one another,
    // the highlight cursor toggles from red to blue if activated when highlight is already active.
    Cursor* useCursor = mHighlightRedCursor;
    if(mHighlightRedCursor == nullptr || mActiveCursor == mHighlightRedCursor)
    {
        useCursor = mHighlightBlueCursor;
    }

    if(useCursor != nullptr)
    {
        mActiveCursor = useCursor;
        mActiveCursor->Activate();
    }
}

void CursorManager::UseWaitCursor()
{
    // "Wait" cursor is just the "loading" cursor, but it doesn't animate.
    if(mWaitCursor != nullptr)
    {
        mActiveCursor = mWaitCursor;
        mActiveCursor->Activate(false);
    }
}

void CursorManager::UseLoadCursor()
{
    if(mWaitCursor != nullptr && mActiveCursor != mWaitCursor)
    {
        mActiveCursor = mWaitCursor;
        mActiveCursor->Activate();
    }
}

void CursorManager::UseCustomCursor(Cursor* cursor)
{
    if(cursor != nullptr && mActiveCursor != cursor)
    {
        mActiveCursor = cursor;
        mActiveCursor->Activate();
    }
}