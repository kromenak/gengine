//
// Clark Kromenaker
//
// A dropdown allows you to select a text option from a list of choices.
//
#pragma once
#include "Actor.h"

#include <functional>
#include <string>
#include <vector>

class RectTransform;
class UIButton;
class UIImage;
class UILabel;

class UIDropdown : public Actor
{
public:
    UIDropdown(Actor* parent);

    void SetMaxVisibleChoices(int max) { mMaxVisibleChoices = max; }

    void SetChoices(const std::vector<std::string>& choices);
    void SetCurrentChoice(const std::string& choice);

    void SetCallback(std::function<void(int)> callback) { mCallback = callback; }

protected:
    void OnUpdate(float deltaTime);

private:
    // The button you press to expand the dropdown.
    UIButton* mExpandButton = nullptr;

    // The text showing the currently selected option.
    UILabel* mCurrentChoiceLabel = nullptr;

    // The box that contains the options you can select.
    // Appears when expand button is pressed.
    RectTransform* mBoxRT = nullptr;

    // The UI elements for each choice.
    struct ChoiceUI
    {
        RectTransform* transform = nullptr;
        UILabel* label = nullptr;
        UIButton* button = nullptr;
    };
    std::vector<ChoiceUI> mChoiceUIs;
    
    // All choices that can be selected.
    std::vector<std::string> mChoices;

    // A callback for when the dropdown selection changes.
    // Passes the index of the choice that was selected, and caller can decide what to do with that.
    std::function<void(int)> mCallback;

    // Max choices to show at one time. Additional choices are available by scrolling. 0 means no maximum...
    int mMaxVisibleChoices = 0;
    
    // This is the offset/index of the selection at the top of the list; mainly used while scrolling.
    int mChoicesOffset = 0;

    void RefreshChoicesUI();

    void OnExpandButtonPressed();
    void OnSelectionPressed(UIButton* button);
};
