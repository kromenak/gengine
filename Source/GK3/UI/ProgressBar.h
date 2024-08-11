//
// Clark Kromenaker
// 
// A popup UI that shows a progress bar and a short message.
//
#pragma once
#include "Actor.h"

#include <string>

#include "LayerManager.h"

class UICanvas;
class UIImage;

class ProgressBar : public Actor
{
public:
    enum class Type
    {
        Generic,
        Save,
        Load
    };

    ProgressBar();

    void Show(Type type);
    void Hide();

    void SetProgress(float fraction);

private:
    // Layer for this UI.
    Layer mLayer;

    // Background of the progress bar.
    // Needed because context text is baked into background based on type of progress bar.
    UIImage* mBackground = nullptr;

    UICanvas* mProgressBarCanvas = nullptr;
    
    UIImage* mProgressBarImage = nullptr;
};