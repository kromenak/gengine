//
// Clark Kromenaker
//
// Loading, managing, and choosing which cursors to use.
//
#pragma once
#include "Type.h"

class Cursor;

class CursorManager
{
    TYPE_DECL_BASE();
public:
    void Init();

    void Update(float deltaTime);

    void UseDefaultCursor();
    void UseHighlightCursor();
    void UseWaitCursor();
    void UseLoadCursor();
    void UseCustomCursor(Cursor* cursor);

private:
    // The cursor that is currently active.
    Cursor* mActiveCursor = nullptr;

    // Cursors assets used in various situations.
    Cursor* mDefaultCursor = nullptr;
    Cursor* mHighlightRedCursor = nullptr;
    Cursor* mHighlightBlueCursor = nullptr;
    Cursor* mWaitCursor = nullptr;
};