//
// Clark Kromenaker
//
// UI for the "Suspects" section of Sidney.
//
#pragma once
#include <string>
#include <vector>

#include "PersistState.h"
#include "SidneyMenuBar.h"
#include "SidneyPopup.h"

class Actor;
class SidneyFiles;
struct SidneyFile;
class UIImage;
class UILabel;
class UINineSlice;
class UITextInput;
class UIVideoImage;

class SidneySuspects
{
public:
    void Init(Actor* parent, SidneyFiles* sidneyFiles);

    void Show();
    void Hide();

    void OpenFile(int fileId);

    void OnUpdate(float deltaTime);

    void OnPersist(PersistState& ps);

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

        // A close-up fingerprint texture used during match analysis for this suspect.
        Texture* matchAnalysisFingerprintTexture = nullptr;

        void OnPersist(PersistState& ps)
        {
            ps.Xfer(PERSIST_VAR(notes));
            ps.Xfer(PERSIST_VAR(linkedFileIds));
            ps.Xfer(PERSIST_VAR(selectedLinkedFileIndex));
        }
    };
    std::vector<SuspectInfo> mSuspectInfos;

    // There're certain linked files that reveal info on this screen. Or grant score.
    // This doesn't seem to be in any game data file, so hardcoding here for now, by index.
    struct LinkedFileEvent
    {
        int fileId = -1;
        std::string scoreName;
        std::string flag;
    };
    std::vector<LinkedFileEvent> mVehicleIdLinkedFiles = {
        { -1, "", "" },                                                             // Buthane, always shows as "Van"
        { 32, "e_sidney_suspect_link_license_buchelli", "IDedBuchelliVehicle" },    // Buchelli
        { 33, "e_sidney_suspect_link_license_emilio",   "IDedEmilioVehicle" },      // Emilio
        { 999, "",                                      "IDedAbbeVehicle" },        // Arnaud, never shows any vehicle
        { 34, "e_sidney_suspect_link_license_howard",   "IDedHowardVehicle" },      // Lady Howard
        { 34, "e_sidney_suspect_link_license_howard",   "IDedEstelleVehicle" },     // Estelle, same file and score event as LH
        { 36, "e_sidney_suspect_link_license_wilkes",   "IDedWilkesVehicle" },      // Wilkes
        { -1, "", "" },                                                             // Larry Chester, always shows "Blue Sedan"
        { -1, "", "" },                                                             // Montreaux, always shows "Auto?"
        { 35, "e_sidney_suspect_link_license_mosely",   "IDedMoselyVehicle" }       // Mosely
    };
    std::vector<LinkedFileEvent> mFingerprintLinkedFiles = {
        { 2, "e_sidney_suspect_link_fingerprint_buthane", "" },     // Buthane
        { 1, "e_sidney_suspect_link_fingerprint_buchelli", "" },    // Buchelli
        { -1, "", "" },                                             // No print for Emilio
        { 0, "e_sidney_suspect_link_fingerprint_abbe", "" },        // Arnaud
        { 4, "e_sidney_suspect_link_fingerprint_howard", "" },      // Lady Howard
        { 3, "e_sidney_suspect_link_fingerprint_estelle", "" },     // Estelle
        { 6, "e_sidney_suspect_link_fingerprint_wilkes", "" },      // Wilkes
        { 8, "e_sidney_suspect_link_fingerprint_larry", "" },       // Larry Chester
        { 5, "e_sidney_suspect_link_fingerprint_montreaux", "" },   // Montreaux
        { 7, "", "" }                                               // Mosely's print isn't added directly by the player, so no score event.
    };

    // The index of the opened suspect, if any.
    int mOpenedSuspectIndex = -1;

    // The ID of the opened file, if any.
    int mOpenedFileId = -1;

    // MATCH ANALYSIS
    Actor* mMatchAnalysisWindow = nullptr;
    UIImage* mMAFingerprintImage = nullptr;
    UIVideoImage* mMAFingerprintVideoImage = nullptr;
    UILabel* mMAActionLabel = nullptr;
    UILabel* mMASuspectLabel = nullptr;
    SidneyButton* mMALinkToSuspectButton = nullptr;
    SidneyButton* mMACloseButton = nullptr;

    void ShowSuspect(int index);
    void ShowFile(SidneyFile* file);

    void RefreshEnabledMenuChoices();

    bool IsSuspectFingerprintLinked(int suspectIndex);

    void OnLinkToSuspectPressed();
    void OnUnlinkToSuspectPressed();

    void OnMatchAnalysisPressed();
    void OnMatchAnalysisCheckSuspect(int currentIndex, int matchIndex);

    void OnLinkedEvidenceSelected(int index);
};