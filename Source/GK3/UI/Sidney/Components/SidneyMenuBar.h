//
// Clark Kromenaker
//
// The menu bar that appears at the top of each subscreen.
// Sometimes has buttons or dropdown menus.
//
#pragma once
#include <functional>
#include <string>
#include <vector>

class Actor;
class Font;
class SidneyButton;
class Texture;
class UIButton;
class UIImage;
class UILabel;

class SidneyMenuBar
{
public:
    void Init(Actor* parent, const std::string& label);
    void Update();

    void SetInteractive(bool interactive) { mInteractive = interactive; }

    // Dropdowns
    void SetFirstDropdownPosition(float position);
    void SetDropdownSpacing(float spacing);
    void AddDropdown(const std::string& label);
    void SetDropdownEnabled(size_t index, bool enabled);

    // Dropdown choices
    void AddDropdownChoice(const std::string& label, const std::function<void()>& pressCallback);
    void AddDropdownChoice(size_t dropdownIndex, const std::string& label, const std::function<void()>& pressCallback);
    void AddDropdownChoiceSeparator();
    void SetDropdownChoiceEnabled(size_t dropdownIndex, size_t choiceIndex, bool enabled);
    void ClearDropdownChoices(size_t dropdownIndex);
    size_t GetDropdownChoiceCount(size_t dropdownIndex) const { return mDropdowns[dropdownIndex].options.size(); }

private:
    // The root of the menu bar hierarchy.
    Actor* mRoot = nullptr;

    // Is the user allowed to interact with the menu bar right now?
    bool mInteractive = true;

    // For placing dropdowns: the next dropdown position, and the desired dropdown spacing.
    // Modifying these while adding dropdowns allows pretty flexible placement.
    float mDropdownSpacing = 15.0f;
    float mNextDropdownPosition = 14.0f;

    // Represents dropdown menus on the menu bar.
    // Hovering an item shows the nested choices. Each choice can be clicked.
    struct Dropdown
    {
        bool enabled = true;

        UIButton* rootButton = nullptr;
        UILabel* rootLabel = nullptr;
        UIImage* rootArrow = nullptr;

        // The background behind the dropdown - most visible if a separator is used.
        UIButton* background = nullptr;

        // The next choice's y-pos, if another one is added.
        float nextChoiceYPos = 0.0f;

        std::vector<SidneyButton*> options;
    };
    std::vector<Dropdown> mDropdowns;

    // Fonts for enabled vs. disabled dropdowns.
    Font* mDropdownFont = nullptr;
    Font* mDropdownDisabledFont = nullptr;

    // Arrow texture for enabled vs. disabled dropdowns.
    Texture* mDropdownArrowTexture = nullptr;
    Texture* mDropdownDisabledArrowTexture = nullptr;
};