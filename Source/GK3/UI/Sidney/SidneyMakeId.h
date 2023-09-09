//
// Clark Kromenaker
//
// UI for the "Make ID" subscreen of Sidney.
//
#pragma once
#include "SidneyMenuBar.h"

class Actor;
class UIImage;
class UILabel;

class SidneyMakeId
{
public:
    void Init(Actor* parent);
    
    void Show();
    void Hide();

    void OnUpdate(float deltaTime);

private:
    // Root of this subscreen.
    Actor* mRoot = nullptr;

    // The menu bar.
    SidneyMenuBar mMenuBar;

    // Identifiers for the ID character/job combo we are making.
    // This is used to populate the labels on the screen.
    std::string mCharacterId = "GAB";
    std::string mJobId = "DOC";

    // The label showing the type of ID to print.
    UILabel* mIdToPrintLabel = nullptr;

    // The image showing the ID preview.
    UIImage* mIdToPrintImage = nullptr;

    void RefreshIdHeader(const std::string& category, const std::string& job);
    void RefreshId();

    void PrintId();
};