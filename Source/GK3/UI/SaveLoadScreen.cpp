#include "SaveLoadScreen.h"

#include "AssetManager.h"
#include "AudioManager.h"
#include "GameProgress.h"
#include "GK3UI.h"
#include "InputManager.h"
#include "Localizer.h"
#include "RectTransform.h"
#include "SaveManager.h"
#include "Texture.h"
#include "UIButton.h"
#include "UIImage.h"
#include "UILabel.h"
#include "UIScrollbar.h"
#include "UITextInput.h"
#include "UIUtil.h"

namespace
{
    const float kSaveListWidth = 396.0f;
    const float kSaveListHeight = 376.0f;

    const float kLeftColumnWidth = 179.0f;
    const float kMiddleColumnWidth = 147.0f;
    const float kRightColumnWidth = 60.0f;
    const float kColumnSeparatorWidth = 5.0f;
    const float kRowHeight = 17.0f;

    const int kMaxSavesOnSingleScreen = 21;
}

SaveLoadScreen::SaveLoadScreen() : Actor("SaveLoadScreen", TransformType::RectTransform),
    mSaveLayer("SaveLayer"),
    mLoadLayer("LoadLayer")
{
    // Save/load layers stop scene SFX/VO, but not music or ambient.
    mSaveLayer.OverrideAudioState(true, true, false);
    mLoadLayer.OverrideAudioState(true, true, false);

    // Needs a high canvas order so it appears above the in-game options bar.
    UI::AddCanvas(this, 21, Color32::Black);

    // Add load/save background image.
    UIImage* background = UI::CreateWidgetActor<UIImage>("Background", this);
    background->SetTexture(gAssetManager.LoadTexture("LOADSAVE.BMP"), true);

    // Create exit button.
    {
        mExitButton = UI::CreateWidgetActor<UIButton>("ExitButton", background);
        mExitButton->SetUpTexture(gAssetManager.LoadTexture("EXITN.BMP"));
        mExitButton->SetDownTexture(gAssetManager.LoadTexture("EXITD.BMP"));
        mExitButton->SetHoverTexture(gAssetManager.LoadTexture("EXITHOV.BMP"));
        mExitButton->SetDisabledTexture(gAssetManager.LoadTexture("EXITDIS.BMP"));

        // Anchor to bottom-right and position based off that.
        mExitButton->GetRectTransform()->SetAnchor(AnchorPreset::BottomLeft);
        mExitButton->GetRectTransform()->SetAnchoredPosition(5, 19.0f);

        mExitButton->SetPressCallback([this](UIButton* button){
            OnExitButtonPressed();
        });
        mExitButton->SetTooltipText("exitloadsave");
    }

    // Create save & load buttons.
    {
        mSaveButton = UI::CreateWidgetActor<UIButton>("SaveButton", background);
        mSaveButton->SetUpTexture(gAssetManager.LoadTexture("SAVEN.BMP"));
        mSaveButton->SetDownTexture(gAssetManager.LoadTexture("SAVED.BMP"));
        mSaveButton->SetHoverTexture(gAssetManager.LoadTexture("SAVEHOV.BMP"));
        mSaveButton->SetDisabledTexture(gAssetManager.LoadTexture("SAVEDIS.BMP"));

        // Anchor to top-right and position based off that.
        mSaveButton->GetRectTransform()->SetAnchor(AnchorPreset::TopRight);
        mSaveButton->GetRectTransform()->SetAnchoredPosition(-22.0f, -31.0f);

        mSaveButton->SetPressCallback([this](UIButton* button){
            OnSaveButtonPressed();
        });
        mSaveButton->SetTooltipText("savegame");
    }
    {
        mLoadButton = UI::CreateWidgetActor<UIButton>("LoadButton", background);
        mLoadButton->SetUpTexture(gAssetManager.LoadTexture("RESTOREN.BMP"));
        mLoadButton->SetDownTexture(gAssetManager.LoadTexture("RESTORED.BMP"));
        mLoadButton->SetHoverTexture(gAssetManager.LoadTexture("RESTOREHOV.BMP"));
        mLoadButton->SetDisabledTexture(gAssetManager.LoadTexture("RESTOREDIS.BMP"));

        // Anchor to top-right and position based off that.
        mLoadButton->GetRectTransform()->SetAnchor(AnchorPreset::TopRight);
        mLoadButton->GetRectTransform()->SetAnchoredPosition(-22.0f, -31.0f);

        mLoadButton->SetPressCallback([this](UIButton* button){
            OnLoadButtonPressed();
        });
        mLoadButton->SetTooltipText("loadgame");
    }

    // Create actor that contains all the list items.
    mListActor = new Actor("SaveList", TransformType::RectTransform);
    mListActor->GetTransform()->SetParent(background->GetRectTransform());
    RectTransform* rt = mListActor->GetComponent<RectTransform>();
    rt->SetAnchor(AnchorPreset::TopLeft);
    rt->SetAnchoredPosition(34.0f, -72.0f);
    rt->SetSizeDelta(kSaveListWidth, kSaveListHeight);

    // Create a highlight for putting behind a list item.
    {
        Actor* highlightActor = new Actor("Highlight", TransformType::RectTransform);
        highlightActor->GetTransform()->SetParent(rt);
        mHighlight = highlightActor->GetComponent<RectTransform>();
        mHighlight->SetAnchor(AnchorPreset::TopLeft);
        mHighlight->SetSizeDelta(kSaveListWidth, kRowHeight);

        UIImage* leftHighlight = UI::CreateWidgetActor<UIImage>("LeftHighlight", highlightActor);
        leftHighlight->SetColor(Color32::Gray);
        leftHighlight->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
        leftHighlight->GetRectTransform()->SetSizeDelta(kLeftColumnWidth, kRowHeight);

        UIImage* middleHighlight = UI::CreateWidgetActor<UIImage>("MiddleHighlight", highlightActor);
        middleHighlight->SetColor(Color32::Gray);
        middleHighlight->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
        middleHighlight->GetRectTransform()->SetAnchoredPosition(kLeftColumnWidth + kColumnSeparatorWidth, 0.0f);
        middleHighlight->GetRectTransform()->SetSizeDelta(kMiddleColumnWidth, kRowHeight);

        UIImage* rightHighlight = UI::CreateWidgetActor<UIImage>("RightHighlight", highlightActor);
        rightHighlight->SetColor(Color32::Gray);
        rightHighlight->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
        rightHighlight->GetRectTransform()->SetAnchoredPosition(kLeftColumnWidth + kMiddleColumnWidth + (2 * kColumnSeparatorWidth), 0.0f);
        rightHighlight->GetRectTransform()->SetSizeDelta(kRightColumnWidth, kRowHeight);
    }

    // Create input field for entering save names.
    {
        mTextInput = UI::CreateWidgetActor<UITextInput>("SaveNameTextInput", mListActor);
        mTextInput->SetFont(gAssetManager.LoadFont("F_SSERIF_T8.FON"));
        mTextInput->SetVerticalAlignment(VerticalAlignment::Top);
        mTextInput->AllowInputToChangeFocus(false); // this text input is always focused if code focuses it
        mTextInput->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
        mTextInput->GetRectTransform()->SetAnchoredPosition(2.0f, 0.0f);
        mTextInput->GetRectTransform()->SetSizeDelta(kLeftColumnWidth, kRowHeight);

        UIImage* caretImage = UI::CreateWidgetActor<UIImage>("Caret", mTextInput);
        caretImage->SetTexture(&Texture::White);
        caretImage->SetColor(Color32(198, 170, 41));
        caretImage->GetRectTransform()->SetAnchor(AnchorPreset::BottomLeft);
        caretImage->GetRectTransform()->SetPivot(1.0f, 0.15f);
        caretImage->GetRectTransform()->SetSizeDelta(2.0f, kRowHeight);
        mTextInput->SetCaret(caretImage);
        mTextInput->SetCaretBlinkInterval(0.5f);
    }

    // Create image for showing save thumbnail.
    {
        mThumbnailImage = UI::CreateWidgetActor<UIImage>("SaveThumbnail", background);
        mThumbnailImage->SetColor(Color32::White);
        mThumbnailImage->GetRectTransform()->SetAnchor(AnchorPreset::TopRight);
        mThumbnailImage->GetRectTransform()->SetAnchoredPosition(-14.0f, -120.0f);
        mThumbnailImage->GetRectTransform()->SetSizeDelta(160.0f, 120.0f);
    }

    // Add scrollbar for when there are too many saves to fit on screen.
    UIScrollbarParams scrollbarParams;
    scrollbarParams.decreaseValueButtonUp = gAssetManager.LoadTexture("SAVELOAD_SCROLLUP_STD.BMP");
    scrollbarParams.decreaseValueButtonDown = gAssetManager.LoadTexture("SAVELOAD_SCROLLUP_DWN.BMP");
    scrollbarParams.increaseValueButtonUp = gAssetManager.LoadTexture("SAVELOAD_SCROLLDN_STD.BMP");
    scrollbarParams.increaseValueButtonDown = gAssetManager.LoadTexture("SAVELOAD_SCROLLDN_DWN.BMP");
    scrollbarParams.scrollbarBacking = gAssetManager.LoadTexture("SAVELOAD_SCROLLBACK.BMP");
    scrollbarParams.handleParams.leftColor = scrollbarParams.handleParams.topColor = scrollbarParams.handleParams.topLeftColor = Color32(181, 125, 0);
    scrollbarParams.handleParams.rightColor = scrollbarParams.handleParams.bottomColor = scrollbarParams.handleParams.topRightColor =
        scrollbarParams.handleParams.bottomRightColor = scrollbarParams.handleParams.bottomLeftColor = Color32(90, 28, 33);
    scrollbarParams.handleParams.centerColor = Color32(123, 77, 8);
    scrollbarParams.handleParams.borderWidth = 2;

    mScrollbar = UI::CreateWidgetActor<UIScrollbar>("Scrollbar", background, scrollbarParams);
    mScrollbar->GetRectTransform()->SetAnchor(AnchorPreset::BottomLeft);
    mScrollbar->GetRectTransform()->SetAnchoredPosition(430.0f, 28.0f);
    mScrollbar->GetRectTransform()->SetSizeDeltaY(385.0f);
    mScrollbar->SetDecreaseValueCallback([this](){
        OnScrollbarUpArrowPressed();
    });
    mScrollbar->SetIncreaseValueCallback([this](){
        OnScrollbarDownArrowPressed();
    });
    mScrollbar->SetValueChangeCallback([this](float value){
        OnScrollbarValueChanged(value);
    });
}

void SaveLoadScreen::ShowSave()
{
    Show(true);
}

void SaveLoadScreen::ShowLoad()
{
    Show(false);
}

void SaveLoadScreen::Hide()
{
    SetActive(false);
    if(mSaveButton->IsEnabled())
    {
        gLayerManager.PopLayer(&mSaveLayer);
    }
    else
    {
        gLayerManager.PopLayer(&mLoadLayer);
    }

    // Make sure text input is no longer focused.
    mTextInput->Unfocus();
}

void SaveLoadScreen::OnUpdate(float deltaTime)
{
    // Other layers can show above this layer, so it must be the top one for keyboard shortcuts to work.
    if(gLayerManager.IsTopLayer(&mSaveLayer) || gLayerManager.IsTopLayer(&mLoadLayer))
    {
        // When up arrow is pressed, move highlight to previous save in the list.
        if(gInputManager.IsKeyLeadingEdge(SDL_SCANCODE_UP) && mSaveIndex > 0)
        {
            // If the previous save is off-screen (due to scrolling), we should scroll one increment so it IS on-screen.
            int listEntryIndex = SaveIndexToListEntryIndex(mSaveIndex - 1);
            if(listEntryIndex < 0)
            {
                OnScrollbarUpArrowPressed();
            }

            // Set new highlighted save.
            SetSelectedSaveIndex(mSaveIndex - 1);
        }

        // When down arrow is pressed, move highlight to next save in the list.
        if(gInputManager.IsKeyLeadingEdge(SDL_SCANCODE_DOWN) && mSaveIndex < mSaveEntryCount - 1)
        {
            // If the next save is off-screen (due to scrolling), scroll down one so it IS on-screen.
            int listEntryIndex = SaveIndexToListEntryIndex(mSaveIndex + 1);
            if(listEntryIndex < 0)
            {
                OnScrollbarDownArrowPressed();
            }

            // Set new highlighted save.
            SetSelectedSaveIndex(mSaveIndex + 1);
        }

        // Escape exits the screen.
        if(gInputManager.IsKeyLeadingEdge(SDL_SCANCODE_ESCAPE))
        {
            mExitButton->AnimatePress();
        }

        // Return acts as a save/load button press.
        if(gInputManager.IsKeyLeadingEdge(SDL_SCANCODE_RETURN))
        {
            if(mSaveButton->IsEnabled())
            {
                mSaveButton->AnimatePress();
            }
            else if(mLoadButton->IsEnabled())
            {
                mLoadButton->AnimatePress();
            }
        }
    }
}

void SaveLoadScreen::Show(bool isSaveScreen)
{
    // Show the screen.
    SetActive(true);
    gLayerManager.PushLayer(isSaveScreen ? &mSaveLayer : &mLoadLayer);

    // Show save or load button depending.
    mSaveButton->SetEnabled(isSaveScreen);
    mLoadButton->SetEnabled(!isSaveScreen);

    // Populate the list of saves.
    PopulateSaveList(true);

    // Make sure text input is disabled to start.
    mTextInput->SetEnabled(false);
}

void SaveLoadScreen::PopulateSaveList(bool justShown)
{
    // Hide all existing list entries.
    for(SaveListEntry& entry : mListEntries)
    {
        entry.button->GetOwner()->SetActive(false);
    }

    // Get existing saves from save manager.
    const std::vector<SaveSummary>& saves = gSaveManager.GetSaves();

    // Load button is disabled if no saves exist.
    mLoadButton->SetCanInteract(!saves.empty());

    // If in save mode, we create one more entry for an "empty" slot.
    int entryCount = saves.size();
    if(mSaveButton->IsEnabled())
    {
        ++entryCount;
    }
    mSaveEntryCount = entryCount;

    // Show the range of saves starting at the scroll ofset and ending either at max saves on screen or entry count (whichever is smaller).
    int startRowIndex = mSaveScrollRowOffset;
    int endRowIndex = Math::Min(mSaveScrollRowOffset + kMaxSavesOnSingleScreen, entryCount) - 1;

    // If the screen has just been shown, it defaults to showing the *bottom* of the list.
    if(justShown)
    {
        startRowIndex = 0;
        endRowIndex = entryCount - 1;
        if(entryCount > kMaxSavesOnSingleScreen)
        {
            startRowIndex = entryCount - kMaxSavesOnSingleScreen;
        }
        mSaveScrollRowOffset = startRowIndex;
    }

    // Create a list entry for each save (and maybe also empty slot).
    int listEntryCounter = 0;
    for(int i = startRowIndex; i <= endRowIndex; ++i)
    {
        SaveListEntry* entry = nullptr;
        if(listEntryCounter < mListEntries.size())
        {
            // Reuse an existing entry.
            entry = &mListEntries[listEntryCounter];
        }
        else
        {
            // Create a new entry.
            mListEntries.emplace_back();
            entry = &mListEntries.back();

            // Parent container for the three labels.
            UIButton* button = UI::CreateWidgetActor<UIButton>("SaveEntry" + std::to_string(i), mListActor);
            button->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
            button->GetRectTransform()->SetSizeDelta(kSaveListWidth, 14.0f);
            button->SetPressCallback([this, listEntryCounter](UIButton* button){
                OnListEntryButtonPressed(listEntryCounter);
            });
            mListEntries.back().button = button;

            // Name label.
            UILabel* nameLabel = UI::CreateWidgetActor<UILabel>("NameLabel", button);
            nameLabel->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
            nameLabel->GetRectTransform()->SetAnchoredPosition(2.0f, -2.0f);
            nameLabel->GetRectTransform()->SetSizeDelta(kLeftColumnWidth, 14.0f);
            nameLabel->SetFont(gAssetManager.LoadFont("F_SSERIF_T8.FON"));
            nameLabel->SetVerticalAlignment(VerticalAlignment::Top);
            mListEntries.back().nameLabel = nameLabel;

            // Day & time label.
            UILabel* dayTimeLabel = UI::CreateWidgetActor<UILabel>("TimeblockLabel", button);
            dayTimeLabel->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
            dayTimeLabel->GetRectTransform()->SetAnchoredPosition(kLeftColumnWidth + kColumnSeparatorWidth + 2.0f, -2.0f);
            dayTimeLabel->GetRectTransform()->SetSizeDelta(kMiddleColumnWidth, 14.0f);
            dayTimeLabel->SetFont(gAssetManager.LoadFont("F_SSERIF_T8.FON"));
            dayTimeLabel->SetVerticalAlignment(VerticalAlignment::Top);
            mListEntries.back().dayTimeLabel = dayTimeLabel;

            // Score label.
            UILabel* scoreLabel = UI::CreateWidgetActor<UILabel>("ScoreLabel", button);
            scoreLabel->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
            scoreLabel->GetRectTransform()->SetAnchoredPosition(kLeftColumnWidth + kMiddleColumnWidth + (2 * kColumnSeparatorWidth) + 2.0f, -2.0f);
            scoreLabel->GetRectTransform()->SetSizeDelta(kRightColumnWidth, 14.0f);
            scoreLabel->SetFont(gAssetManager.LoadFont("F_SSERIF_T8.FON"));
            scoreLabel->SetVerticalAlignment(VerticalAlignment::Top);
            mListEntries.back().scoreLabel = scoreLabel;
        }

        // Make sure the entry is active (in case reusing an old one).
        entry->button->GetOwner()->SetActive(true);

        // Populate entry with save details.
        if(i < saves.size())
        {
            const SaveSummary& save = saves[i];
            entry->nameLabel->SetText(save.persistHeader.userDescription);
            entry->dayTimeLabel->SetText(gGameProgress.GetTimeblockDisplayName(save.persistHeader.timeblock));
            entry->scoreLabel->SetText(StringUtil::Format("%03i / %03i", save.persistHeader.score, save.persistHeader.maxScore));
        }
        else // empty slot
        {
            entry->nameLabel->SetText("[Empty]");
        }

        // Position appropriately in list.
        entry->button->GetRectTransform()->SetAnchoredPosition(0.0f, listEntryCounter * -kRowHeight);

        // We used one list entry, so increment to next one.
        ++listEntryCounter;
    }

    // When first shown, highlight the last save in the list.
    if(justShown)
    {
        SetSelectedSaveIndex(endRowIndex);
    }
    else
    {
        SetSelectedSaveIndex(mSaveIndex);
    }

    // Update scrollbar to reflect the list size/state.
    int rowsNotOnScreen = entryCount - kMaxSavesOnSingleScreen;
    mMaxSaveScrollRowOffset = rowsNotOnScreen;
    if(rowsNotOnScreen > 0)
    {
        // Size handle relative to number of entries not on screen. More entries = smaller handle.
        float normalizedScrollbarHandleSize = 1.0f / rowsNotOnScreen;
        mScrollbar->SetHandleNormalizedSize(normalizedScrollbarHandleSize);

        // Make sure scrollbar is in the correct spot.
        mScrollbar->SetValueSilently(static_cast<float>(mSaveScrollRowOffset) / rowsNotOnScreen);
    }
    mScrollbar->SetCanInteract(rowsNotOnScreen > 0);
}

int SaveLoadScreen::SaveIndexToListEntryIndex(int saveIndex)
{
    int listEntryIndex = saveIndex - mSaveScrollRowOffset;
    if(listEntryIndex >= 0 && listEntryIndex < mListEntries.size() && mListEntries[listEntryIndex].button->GetOwner()->IsActive())
    {
        return listEntryIndex;
    }
    return -1;
}

int SaveLoadScreen::ListEntryIndexToSaveIndex(int listEntryIndex)
{
    return listEntryIndex + mSaveScrollRowOffset;
}

void SaveLoadScreen::SetSelectedSaveIndex(int saveIndex)
{
    mSaveIndex = saveIndex;

    int listEntryIndex = SaveIndexToListEntryIndex(mSaveIndex);
    if(listEntryIndex >= 0)
    {
        // Update highlight position.
        mHighlight->GetOwner()->SetActive(true);
        mHighlight->SetAnchoredPosition(0.0f, listEntryIndex * -kRowHeight);
    }
    else
    {
        mHighlight->GetOwner()->SetActive(false);
    }

    // Update the thumbnail.
    const std::vector<SaveSummary>& saves = gSaveManager.GetSaves();
    if(mSaveIndex >= 0 && mSaveIndex < saves.size() && saves[mSaveIndex].persistHeader.thumbnailTexture != nullptr)
    {
        mThumbnailImage->SetTexture(saves[mSaveIndex].persistHeader.thumbnailTexture.get());
    }
    else
    {
        mThumbnailImage->SetTexture(&Texture::Black);
    }
}

void SaveLoadScreen::ShowSaveOverwriteConfirm(int listEntryIndex)
{
    gGK3UI.ShowConfirmPopup(gLocalizer.GetText("OverwriteSave"), [this, listEntryIndex](bool wantsOverwrite){
        if(wantsOverwrite)
        {
            ActivateTextInput(listEntryIndex);
        }
    });
}

void SaveLoadScreen::ActivateTextInput(int listEntryIndex)
{
    // Show text input over the name label.
    mTextInput->GetRectTransform()->SetAnchoredPosition(2.0f, (listEntryIndex * -kRowHeight) - 2.0f);
    mTextInput->SetEnabled(true);

    // If this is the "empty" slot at the end of the list, clear the text to enter a full name.
    // Otherwise, the text stays so the player can keep it if they want.
    int saveIndex = ListEntryIndexToSaveIndex(listEntryIndex);
    if(saveIndex >= gSaveManager.GetSaves().size())
    {
        mTextInput->Clear();
        mListEntries[listEntryIndex].nameLabel->SetText("");
    }
    else
    {
        // Show the previously entered text in text input.
        mTextInput->SetText(mListEntries[listEntryIndex].nameLabel->GetText());
        mListEntries[listEntryIndex].nameLabel->SetText("");
    }

    // Player must enter text or exit out at this point.
    mTextInput->Focus();

    // The scrollbar is also no longer interactive at this point.
    mScrollbar->SetCanInteract(false);
}

void SaveLoadScreen::OnListEntryButtonPressed(int listEntryIndex)
{
    // When text input is enabled, pressing entry buttons has no effect anymore.
    if(mTextInput->IsEnabled())
    {
        return;
    }

    // In save mode, if we select a slot that's already highlighted, we try to save to that slot.
    if(listEntryIndex == SaveIndexToListEntryIndex(mSaveIndex) && mSaveButton->IsEnabled())
    {
        // If this is the empty slot, we can activate the text input right away.
        // Otherwise, show a confirm popup first.
        if(mSaveIndex == gSaveManager.GetSaves().size())
        {
            ActivateTextInput(listEntryIndex);
        }
        else
        {
            ShowSaveOverwriteConfirm(listEntryIndex);
        }
    }

    // Highlight selected button index.
    SetSelectedSaveIndex(ListEntryIndexToSaveIndex(listEntryIndex));
}

void SaveLoadScreen::OnSaveButtonPressed()
{
    gAudioManager.PlaySFX(gAssetManager.LoadAudio("SIDBUTN-1.WAV"));
    if(mSaveIndex < gSaveManager.GetSaves().size())
    {
        if(!mTextInput->IsEnabled())
        {
            ShowSaveOverwriteConfirm(SaveIndexToListEntryIndex(mSaveIndex));
        }
        else
        {
            Hide();
            gSaveManager.Save(mTextInput->GetText(), mSaveIndex);
            mTextInput->Unfocus();
        }
    }
    else // new save in empty slot
    {
        if(!mTextInput->IsEnabled())
        {
            ActivateTextInput(SaveIndexToListEntryIndex(mSaveIndex));
        }
        else
        {
            Hide();
            gSaveManager.Save(mTextInput->GetText());
        }
    }
}

void SaveLoadScreen::OnLoadButtonPressed()
{
    gAudioManager.PlaySFX(gAssetManager.LoadAudio("SIDBUTN-1.WAV"));

    // Hide this screen. Also, make sure title screen is hidden (in case loading from title screen).
    Hide();
    gGK3UI.HideTitleScreen(); //TODO: Maybe a better way to do this via the layer system?

    gSaveManager.Load(gSaveManager.GetSaves()[mSaveIndex].filePath);
}

void SaveLoadScreen::OnExitButtonPressed()
{
    gAudioManager.PlaySFX(gAssetManager.LoadAudio("SIDBUTN-1.WAV"));
    Hide();
}

void SaveLoadScreen::OnScrollbarUpArrowPressed()
{
    if(mSaveScrollRowOffset > 0)
    {
        --mSaveScrollRowOffset;
    }
    PopulateSaveList(false);
}

void SaveLoadScreen::OnScrollbarDownArrowPressed()
{
    if(mSaveScrollRowOffset < mMaxSaveScrollRowOffset)
    {
        ++mSaveScrollRowOffset;
    }
    PopulateSaveList(false);
}

void SaveLoadScreen::OnScrollbarValueChanged(float value)
{
    // Similar to inventory screen, this scroll bar isn't continuous - it only affects the scroll list when you hit the next threshold.
    // So first thing, figure out which threshold this value corresponds with.
    int oldRowOffset = mSaveScrollRowOffset;
    for(int i = 0; i <= mMaxSaveScrollRowOffset; ++i)
    {
        float normalizedValue = static_cast<float>(i) / mMaxSaveScrollRowOffset;
        if(value >= normalizedValue)
        {
            // The scrollbar value is more than the value for this threshold, so we'll use this offset.
            mSaveScrollRowOffset = i;
        }
        else
        {
            // The scrollbar is less than this threshold, so we don't need to iterate anymore.
            break;
        }
    }

    // If the offset changed due to changing the scrollbar value, refresh the layout.
    if(mSaveScrollRowOffset != oldRowOffset)
    {
        PopulateSaveList(false);
    }
}