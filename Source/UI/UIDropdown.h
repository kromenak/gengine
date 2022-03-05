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
class UICanvas;
class UIImage;
class UILabel;

class UIDropdown : public Actor
{
public:
    UIDropdown(UICanvas& canvas);

    void SetChoices(const std::vector<std::string>& choices);
    void SetCallback(std::function<void(int)> callback) { mCallback = callback; }

private:
    // The canvas this dropdown is part of.
    UICanvas& mCanvas;

    // The button you press to expand the dropdown.
    UIButton* mExpandButton = nullptr;

    // The text showing the currently selected option.
    UILabel* mCurrentChoiceLabel = nullptr;

    // The box that contains the options you can select.
    // Appears when expand button is pressed.
    RectTransform* mBoxRT = nullptr;

    // The options you can select from the dropdown box.
    struct Selection
    {
        RectTransform* transform = nullptr;
        UILabel* label = nullptr;
        UIButton* button = nullptr;
    };
    std::vector<Selection> mSelections;
    
    // All choices that can be selected.
    std::vector<std::string> mChoices;

    // A callback for when the dropdown selection changes.
    // Passes the index of the choice that was selected, and caller can decide what to do with that.
    std::function<void(int)> mCallback;

    void OnExpandButtonPressed();
};
