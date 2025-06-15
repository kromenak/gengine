//
// Clark Kromenaker
//
// A popup UI that shows a progress bar and a short message.
//
#pragma once
#include "Actor.h"

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
    void ShowFakeProgress(float duration);

protected:
    void OnUpdate(float deltaTime) override;

private:
    // Width & height of the progress bar.
    static constexpr float kProgressBarWidth = 513.0f;
    static constexpr float kProgressBarHeight = 50.0f;

    // Layer for this UI.
    Layer mLayer;

    // Background of the progress bar.
    // Needed because context text is baked into background based on type of progress bar.
    UIImage* mBackground = nullptr;

    // A masked canvas that contains the progress bar image.
    // We need to cache this because resizing the canvas gets the progress bar effect.
    UICanvas* mProgressBarCanvas = nullptr;

    // The image inside the progress bar.
    // Needed to adjust y-pos so we get different progress bar images each time.
    UIImage* mProgressBarImage = nullptr;

    // Ideally, when you show a progress bar, you want to *actually* be able to track the progress of something.
    // But if it's just for appearances sake, or nostalgia, or the thing being tracked can't be easily tracked, you can fake the progress.
    float mFakeProgressDuration = 0.0f;
    float mFakeProgressTimer = 0.0f;
};