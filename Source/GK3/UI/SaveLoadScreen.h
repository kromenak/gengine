#pragma once
#include "Actor.h"

#include "LayerManager.h"

class RectTransform;
class UIButton;
class UIImage;
class UILabel;
class UIScrollbar;
class UITextInput;

class SaveLoadScreen : public Actor
{
public:
    SaveLoadScreen();

    void ShowSave();
    void ShowLoad();
    void Hide();

protected:
    void OnUpdate(float deltaTime) override;

private:
    // Layers for save and load, pushed when entering this screen.
    Layer mSaveLayer;
    Layer mLoadLayer;

    // The exit button.
    UIButton* mExitButton = nullptr;

    // Save and load buttons, appropriate one is enabled based on whether you want to save or load.
    UIButton* mSaveButton = nullptr;
    UIButton* mLoadButton = nullptr;

    // A container for all the items in the list of save games.
    Actor* mListActor = nullptr;

    // A highlight that appears behind a selected save game.
    RectTransform* mHighlight = nullptr;

    // A text input field that is used to enter a save name.
    UITextInput* mTextInput = nullptr;

    // An image to show the save file thumbnail.
    UIImage* mThumbnailImage = nullptr;

    // For each existing save, entries in the list.
    struct SaveListEntry
    {
        UIButton* button = nullptr;
        UILabel* nameLabel = nullptr;
        UILabel* dayTimeLabel = nullptr;
        UILabel* scoreLabel = nullptr;
    };
    std::vector<SaveListEntry> mListEntries;

    // The save index that is currently chosen/active.
    int mSaveIndex = -1;

    // The number of entries in the save list.
    int mSaveEntryCount = 0;

    // When the list of saves gets too long, we need a scrollbar!
    // The index keeps track of which save is at the top of the screen.
    UIScrollbar* mScrollbar = nullptr;
    int mSaveScrollRowOffset = 0;
    int mMaxSaveScrollRowOffset = 0;

    void Show(bool isSaveScreen);

    void PopulateSaveList(bool justShown);

    int SaveIndexToListEntryIndex(int saveIndex);
    int ListEntryIndexToSaveIndex(int listEntryIndex);

    void SetSelectedSaveIndex(int saveIndex);
    void ShowSaveOverwriteConfirm(int listEntryIndex);
    void ActivateTextInput(int listEntryIndex);
    void OnListEntryButtonPressed(int listEntryIndex);

    void OnSaveButtonPressed();
    void OnLoadButtonPressed();
    void OnExitButtonPressed();

    void OnScrollbarUpArrowPressed();
    void OnScrollbarDownArrowPressed();
    void OnScrollbarValueChanged(float value);
};