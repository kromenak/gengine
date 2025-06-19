//
// Clark Kromenaker
//
// Tracks whether a hint is available, and if so, provides a way to show the hint.
//
#pragma once

class HintManager
{
public:
    bool IsHintAvailable() const;
    void ShowHint();

private:

};

extern HintManager gHintManager;