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
class SidneyButton;
class UIButton;

class SidneyMenuBar
{
public:
    void Init(Actor* parent, const std::string& label, float labelWidth);

    void Update();

    void AddDropdown(const std::string& label);
    void AddDropdownChoice(const std::string& label, std::function<void()> pressCallback);

private:
    // The root of the menu bar hierarchy.
    Actor* mRoot = nullptr;

    const float kFirstMenuItemPos = 14.0f;
    const float kMenuItemSpacing = 15.0f;
    float mNextMenuItemPos = kFirstMenuItemPos;

    // Represents dropdown menus on the menu bar.
    // Hovering an item shows the nested choices. Each choice can be clicked.
    struct Dropdown
    {
        UIButton* root = nullptr;
        std::vector<SidneyButton*> options;
    };
    std::vector<Dropdown> mDropdowns;
};