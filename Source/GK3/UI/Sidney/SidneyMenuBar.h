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
    void Init(Actor* parent, const std::string& label, float labelWidth);
    void Update();

    void SetFirstDropdownPosition(float position);
    void SetDropdownSpacing(float spacing);

    void AddDropdown(const std::string& label);
    void AddDropdownChoice(const std::string& label, std::function<void()> pressCallback);

    void SetDropdownEnabled(size_t index, bool enabled);

private:
    // The root of the menu bar hierarchy.
    Actor* mRoot = nullptr;

    // For placing dropdowns: the next dropdown position, and the desired dropdown spacing.
    // Modifying these while adding dropdowns allows pretty flexible placement.
    float mDropdownSpacing = 15.0f;
    float mNextDropdownPosition = 14.0f;

    // Represents dropdown menus on the menu bar.
    // Hovering an item shows the nested choices. Each choice can be clicked.
    struct Dropdown
    {
        UIButton* rootButton = nullptr;
        UILabel* rootLabel = nullptr;
        UIImage* rootArrow = nullptr;

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