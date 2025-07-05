//
// Clark Kromenaker
//
// UI for the "Translate" subscreen of Sidney.
//
#pragma once
#include "SidneyMenuBar.h"

class Actor;
class PersistState;
class SidneyFiles;
class SidneyPopup;
class UILabel;
class UIScrollRect;

class SidneyTranslate
{
public:
    void Init(Actor* parent, SidneyFiles* sidneyFiles);

    void Show(int openFileId = -1);
    void Hide();

    void OnUpdate(float deltaTime);

    void OnPersist(PersistState& ps);

private:
    // References to Sidney modules.
    SidneyFiles* mSidneyFiles = nullptr;

    // Root of this subscreen.
    Actor* mRoot = nullptr;

    // The menu bar.
    SidneyMenuBar mMenuBar;

    // ID of file currently being translated.
    int mTranslateFileId = -1;

    // A popup for displaying messages on this screen.
    SidneyPopup* mPopup = nullptr;

    // The translate window, which is the main thing on this screen.
    Actor* mTranslateWindow = nullptr;

    // Header label for what's being translated.
    UILabel* mTranslateHeaderLabel = nullptr;

    // Scroll rect containing the translated text.
    UIScrollRect* mTranslateTextScrollRect = nullptr;

    // Actually displays the translated text.
    UILabel* mTranslateTextLabel = nullptr;

    // The language we want to translate from and to.
    enum class Language
    {
        English,
        Latin,
        French,
        Italian
    };
    Language mFromLanguage = Language::English;
    Language mToLanguage = Language::Latin;

    // Butons in Sidney UI for selecting from/to languages.
    SidneyButton* mFromButtons[4] = { 0 };
    SidneyButton* mToButtons[4] = { 0 };

    // Known translation actions in the game.
    struct TranslationAction
    {
        // File being translated.
        int fileId = -1;

        // Language to translate this file from.
        Language language;

        // A prefix used to generate the unlocalized and localized text.
        std::string locPrefix;

        // In some cases, a different loc prefix is used if you successfully translated an item before.
        std::string alreadyTranslatedLocPrefix;

        // If set, this flag is set upon successful translation.
        std::string progressFlag;

        // If set, this score event triggers upon successful translation.
        std::string scoreEvent;
    };
    std::vector<TranslationAction> mTranslations;

    // Keeps track of whether we translated the current file yet.
    bool mPerformedTranslation = false;

    TranslationAction* GetTranslationAction(int fileId);
    std::string GetLocKeyForLanguage(Language language);
    std::string GenerateBodyText(const std::string& locPrefix);
    void AppendTranslatedText(TranslationAction* action);
    void AppendTranslatedText(const std::string& text);

    void OpenFile(int fileId);
    void OnTranslateButtonPressed();

    void AskToAddMissingWord();
    void PromptForMissingWord();
};