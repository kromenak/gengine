//
// Clark Kromenaker
//
// This is a popup that shows Gabe/Grace typing some input into a popup.
// It's "fake" because the in-game character types, not the player - it's all performative.
//
// It's non-interactive, but it's complex enough and used in multiple spots in the UI.
// So a helper class makes sense.
//
#pragma once
#include "Actor.h"

#include <functional>
#include <string>

#include "Vector2.h"

class SidneyButton;
class UILabel;

class SidneyFakeInputPopup : public Actor
{
public:
    SidneyFakeInputPopup(Actor* parent, const std::string& name);

    void Show(const std::string& headerText, const std::string& promptText, const std::string& textToType, std::function<void()> doneCallback = nullptr);
    void Hide();

protected:
    void OnUpdate(float deltaTime) override;

private:
    // A header explaining what this popup is (e.g. "Input Complete" or "Add Text Label").
    UILabel* mHeaderLabel = nullptr;

    // A label that displays what is being inputed (e.g. "File Name:" or "Enter Text:").
    UILabel* mPromptLabel = nullptr;

    // The input that is being entered.
    UILabel* mInputLabel = nullptr;

    // A button to confirm the input.
    SidneyButton* mOKButton = nullptr;

    // The in-game character "types" the name of the file before adding it.
    // We use these variables to track the state of the text being typed.
    std::string mTextToType;
    int mTextToTypeIndex = -1;

    // To simulate distinct keystrokes while typing, we use a timer with randomized duration.
    const Vector2 kMinMaxTypeInterval = Vector2(0.2f, 0.5f);
    float mTextToTypeTimer = 0.0f;

    // A callback to fire when the fake typing is done.
    std::function<void()> mDoneCallback = nullptr;
};