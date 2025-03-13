#pragma once
#include "Actor.h"

#include "LayerManager.h"

class RectTransform;
class UIButton;
class UIImage;
class UILabel;
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

    void PopulateSaveList();

    void SetSelectedSaveIndex(int index);
    void ActivateTextInput(int index);
    void OnEntryButtonPressed(int index);

    void OnSaveButtonPressed();
    void OnLoadButtonPressed();
    void OnExitButtonPressed();
};