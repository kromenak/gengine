//
// Clark Kromenaker
// 
// UI for the "Suspects" section of Sidney.
//
#pragma once
#include <string>
#include <vector>

#include "SidneyMenuBar.h"
#include "SidneyPopup.h"
#include "StringUtil.h"

class Actor;
class SidneyFiles;
struct SidneyFile;
class UIImage;
class UILabel;
class UINineSlice;
class UITextInput;

class SidneySuspects
{
public:
    void Init(Actor* parent, SidneyFiles* sidneyFiles);

    void Show();
    void Hide();

    void OnUpdate(float deltaTime);

private:
    // Ref to the file system in Sidney.
    SidneyFiles* mFiles = nullptr;

    // The root of this screen.
    Actor* mRoot = nullptr;

    // The menu bar.
    SidneyMenuBar mMenuBar;

    // A popup to display occasional messages.
    SidneyPopup* mPopup = nullptr;

    // The window that shows suspect data.
    Actor* mSuspectDataWindow = nullptr;

    // The labels that show suspect name/nationality/vehicle ID.
    UILabel* mNameLabel = nullptr;
    UILabel* mNationalityLabel = nullptr;
    UILabel* mVehicleLabel = nullptr;

    // An image that shows the fingerprint of the suspect.
    UIImage* mFingerprintImage = nullptr;

    // Input for notes.
    UITextInput* mNotesInput = nullptr;

    // Each suspect can have up to six linked evidence items.
    // These are the UI for each of those six items.
    static const int kMaxLinkedEvidence = 6;
    struct LinkedEvidenceUI
    {
        Actor* root = nullptr;
        UINineSlice* border = nullptr;
        UILabel* label = nullptr;
        UIImage* image = nullptr;
    };
    LinkedEvidenceUI mLinkedEvidenceItems[kMaxLinkedEvidence];

    // A little window that shows a preview of the opened file.
    Actor* mFilePreviewWindow = nullptr;
    UILabel* mFilePreviewHeaderLabel = nullptr;
    UIImage* mFilePreviewImage = nullptr;

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
        std::vector<int> linkedFileIds;

        // Which linked file is selected.
        int selectedLinkedFileIndex = -1;
    };
    std::vector<SuspectInfo> mSuspectInfos;

    // There're certain linked files that reveal info on this screen. Or grant score.
    // This doesn't seem to be in any game data file, so hardcoding here for now, by index.
    std::vector<int> mVehicleIdLinkedFiles = {
        -1,  // Buthane, always shows as "Van"
        32, 
        33,
        999, // Arnaud, never shows anything
        34,
        34,
        36,
        -1,  // Larry Chester, always shows "Blue Sedan"
        -1,  // Montreaux, always shows "Auto?"
        35
    };
    std::vector<int> mFingerprintLinkedFiles = {
        2,
        1,
        -1,  // No print for Emilio
        0,
        4,
        3,
        6,
        8,
        5,
        7
    };

    // The index of the opened suspect, if any.
    int mOpenedSuspectIndex = -1;

    // The ID of the opened file, if any.
    int mOpenedFileId = -1;

    void ShowSuspect(int index);
    void ShowFile(SidneyFile* file);

    void RefreshEnabledMenuChoices();

    void OnLinkToSuspectPressed();
    void OnUnlinkToSuspectPressed();
    void OnMatchAnalysisPressed();

    void OnLinkedEvidenceSelected(int index);
};