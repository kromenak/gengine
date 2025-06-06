//
// Clark Kromenaker
//
// Loading, managing, and choosing which cursors to use.
//
#pragma once

class Cursor;

class CursorManager
{
public:
    void Init();

    void Update(float deltaTime);

    void UseDefaultCursor();
    void UseRedHighlightCursor(int priority = 0);
    void UseHighlightCursor(int priority = 0);
    void UseWaitCursor();
    void UseLoadCursor();
    void UseCustomCursor(Cursor* cursor, int priority = 0);

private:
    // The cursor that is active and rendering.
    Cursor* mActiveCursor = nullptr;

    // The desired cursor, based on priority amongst potentially many competing game systems.
    // Tracking a desired cursor with priority allows many different systems to request a cursor, but for only one to win out.
    Cursor* mDesiredCursor = nullptr;
    int mDesiredCursorPriority = -1;
    bool mDesiredCursorAnimate = true;

    // Cursors assets used in various situations.
    Cursor* mDefaultCursor = nullptr;
    Cursor* mHighlightRedCursor = nullptr;
    Cursor* mHighlightBlueCursor = nullptr;
    Cursor* mWaitCursor = nullptr;

    void SetDesiredCursor(Cursor* cursor,int priority, bool animate = true);
};

extern CursorManager gCursorManager;