//
// Clark Kromenaker
// 
// UI for the "Suspects" section of Sidney.
//
#pragma once
#include <string>
#include <vector>

#include "SidneyMenubar.h"
#include "StringUtil.h"

class Actor;

class SidneySuspects
{
public:
    void Init(Actor* parent);

    void Show();
    void Hide();

    void OnUpdate(float deltaTime);

private:
    // The root of this screen.
    Actor* mRoot = nullptr;

    // The menu bar.
    SidneyMenuBar mMenuBar;

    // For each suspect, we keep track of known info and linked files.
    struct SuspectInfo
    {
        // The suspects name, nationality, and vehicle ID.
        std::string name;
        std::string nationality;
        std::string vehicleId;

        // The player can also enter their own notes!
        std::string notes;

        // Any linked files.
        std::vector<int> mLinkedFileIds;
    };
    std::vector<SuspectInfo> mSuspectInfos;

    void ShowSuspect(int index);
};