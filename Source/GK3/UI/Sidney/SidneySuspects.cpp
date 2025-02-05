#include "SidneySuspects.h"

#include "Actor.h"
#include "SidneyUtil.h"

void SidneySuspects::Init(Actor* parent)
{
    // Add background. This will also be the root for this screen.
    mRoot = SidneyUtil::CreateBackground(parent);
    mRoot->SetName("Suspects");

    // Add main menu button.
    SidneyUtil::CreateMainMenuButton(mRoot, [&](){
        Hide();
    });

    // Add menu bar.
    mMenuBar.Init(mRoot, SidneyUtil::GetSuspectsLocalizer().GetText("ScreenName"), 124.0f);

    // File menu.
    mMenuBar.AddDropdown(SidneyUtil::GetSuspectsLocalizer().GetText("Menu1Name"));
    mMenuBar.AddDropdownChoice(SidneyUtil::GetSuspectsLocalizer().GetText("MenuItem1"), [](){

    });

    // Suspects menu.
    mMenuBar.AddDropdown(SidneyUtil::GetSuspectsLocalizer().GetText("Menu2Name"));
    // Suspects are added in the show function below!

    // Link menu.
    mMenuBar.AddDropdown(SidneyUtil::GetSuspectsLocalizer().GetText("Menu3Name"));
    mMenuBar.AddDropdownChoice(SidneyUtil::GetSuspectsLocalizer().GetText("Menu3Item1"), [](){
        // Link to suspect.
    });
    mMenuBar.AddDropdownChoice(SidneyUtil::GetSuspectsLocalizer().GetText("Menu3Item2"), [](){
        // Un-link file.
    });
    //TODO: separator
    mMenuBar.AddDropdownChoice(SidneyUtil::GetSuspectsLocalizer().GetText("Menu3Item4"), [](){
        // Match analysis.
    });

    // Populate the suspect data.
    // We can do this in an unusual way: rather than having a dedicated "SUSPECTS.TXT" or whatever, the data is in the localizer.
    // So, we can just iterate and retrieve from there.
    for(int i = 1; i <= 10; ++i)
    {
        mSuspectInfos.emplace_back();
        mSuspectInfos.back().name = SidneyUtil::GetSuspectsLocalizer().GetText("Name" + std::to_string(i));
        mSuspectInfos.back().nationality = SidneyUtil::GetSuspectsLocalizer().GetText("Nationality" + std::to_string(i));
        mSuspectInfos.back().vehicleId = SidneyUtil::GetSuspectsLocalizer().GetText("VehicleID" + std::to_string(i));
    }

    // Hide by default.
    Hide();
}

void SidneySuspects::Show()
{
    // At the start of the game, the dropdown doesn't show all suspects. You add a few later.
    // First, add the initial bunch.
    if(mMenuBar.GetDropdownChoiceCount(1) == 0)
    {
        for(int i = 0; i < 8; ++i)
        {
            mMenuBar.AddDropdownChoice(1, SidneyUtil::GetSuspectsLocalizer().GetText("Menu2Item" + std::to_string(i + 1)), [this, i](){
                ShowSuspect(i);
            });
        }
    }

    // Show the root.
    mRoot->SetActive(true);
}

void SidneySuspects::Hide()
{
    // Hide the root.
    mRoot->SetActive(false);
}

void SidneySuspects::OnUpdate(float deltaTime)
{
    mMenuBar.Update();
}

void SidneySuspects::ShowSuspect(int index)
{
    printf("Show suspect %s\n", mSuspectInfos[index].name.c_str());
}