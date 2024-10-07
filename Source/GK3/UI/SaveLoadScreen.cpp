#include "SaveLoadScreen.h"

#include "AssetManager.h"
#include "AudioManager.h"
#include "GameProgress.h"
#include "Localizer.h"
#include "LocationManager.h"
#include "RectTransform.h"
#include "SaveManager.h"
#include "Texture.h"
#include "UIButton.h"
#include "UICanvas.h"
#include "UIImage.h"
#include "UILabel.h"
#include "UITextInput.h"
#include "UIUtil.h"
#include "Window.h"

namespace
{
    const float kSaveListWidth = 396.0f;
    const float kSaveListHeight = 376.0f;

    const float kLeftColumnWidth = 179.0f;
    const float kMiddleColumnWidth = 147.0f;
    const float kRightColumnWidth = 60.0f;
    const float kColumnSeparatorWidth = 5.0f;
    const float kRowHeight = 16.0f;
}

SaveLoadScreen::SaveLoadScreen() : Actor(TransformType::RectTransform),
    mSaveLayer("SaveLayer"),
    mLoadLayer("LoadLayer")
{
    // Save/load layers stop scene SFX/VO, but not music or ambient.
    mSaveLayer.OverrideAudioState(true, true, false);
    mLoadLayer.OverrideAudioState(true, true, false);

    // Needs a high canvas order so it appears above the in-game options bar.
    UIUtil::AddColorCanvas(this, 1, Color32::Black);

    // Add load/save background image.
    UIImage* background = UIUtil::NewUIActorWithWidget<UIImage>(this);
    background->SetTexture(gAssetManager.LoadTexture("LOADSAVE.BMP"), true);

    // Create exit button.
    {
        UIButton* button = UIUtil::NewUIActorWithWidget<UIButton>(background->GetOwner());
        button->SetUpTexture(gAssetManager.LoadTexture("EXITN.BMP"));
        button->SetDownTexture(gAssetManager.LoadTexture("EXITD.BMP"));
        button->SetHoverTexture(gAssetManager.LoadTexture("EXITHOV.BMP"));
        button->SetDisabledTexture(gAssetManager.LoadTexture("EXITDIS.BMP"));

        // Anchor to bottom-right and position based off that.
        button->GetRectTransform()->SetAnchor(AnchorPreset::BottomLeft);
        button->GetRectTransform()->SetAnchoredPosition(5, 19.0f);

        button->SetPressCallback([this](UIButton* button){
            OnExitButtonPressed();
        });
    }

    // Create save & load buttons.
    {
        mSaveButton = UIUtil::NewUIActorWithWidget<UIButton>(background->GetOwner());
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
    }
    {
        mLoadButton = UIUtil::NewUIActorWithWidget<UIButton>(background->GetOwner());
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
    }

    // Create actor that contains all the list items.
    mListActor = new Actor(TransformType::RectTransform);
    mListActor->GetTransform()->SetParent(background->GetRectTransform());
    RectTransform* rt = mListActor->GetComponent<RectTransform>();
    rt->SetAnchor(AnchorPreset::TopLeft);
    rt->SetAnchoredPosition(34.0f, -72.0f);
    rt->SetSizeDelta(kSaveListWidth, kSaveListHeight);

    // Create a highlight for putting behind a list item.
    {
        Actor* highlightActor = new Actor(TransformType::RectTransform);
        highlightActor->GetTransform()->SetParent(rt);
        mHighlight = highlightActor->GetComponent<RectTransform>();
        mHighlight->SetAnchor(AnchorPreset::TopLeft);
        mHighlight->SetSizeDelta(kSaveListWidth, kRowHeight);

        UIImage* leftHighlight = UIUtil::NewUIActorWithWidget<UIImage>(highlightActor);
        leftHighlight->SetColor(Color32::Gray);
        leftHighlight->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
        leftHighlight->GetRectTransform()->SetSizeDelta(kLeftColumnWidth, kRowHeight);

        UIImage* middleHighlight = UIUtil::NewUIActorWithWidget<UIImage>(highlightActor);
        middleHighlight->SetColor(Color32::Gray);
        middleHighlight->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
        middleHighlight->GetRectTransform()->SetAnchoredPosition(kLeftColumnWidth + kColumnSeparatorWidth, 0.0f);
        middleHighlight->GetRectTransform()->SetSizeDelta(kMiddleColumnWidth, kRowHeight);

        UIImage* rightHighlight = UIUtil::NewUIActorWithWidget<UIImage>(highlightActor);
        rightHighlight->SetColor(Color32::Gray);
        rightHighlight->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
        rightHighlight->GetRectTransform()->SetAnchoredPosition(kLeftColumnWidth + kMiddleColumnWidth + (2 * kColumnSeparatorWidth), 0.0f);
        rightHighlight->GetRectTransform()->SetSizeDelta(kRightColumnWidth, kRowHeight);

        mHighlight->SetEnabled(false);
    }

    // Create input field for entering save names.
    {
        mTextInput = UIUtil::NewUIActorWithWidget<UITextInput>(mListActor);
        mTextInput->SetFont(gAssetManager.LoadFont("F_SSERIF_T8.FON"));
        mTextInput->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
        mTextInput->GetRectTransform()->SetSizeDelta(kLeftColumnWidth, kRowHeight);
    }
}

void SaveLoadScreen::ShowSave()
{
    SetActive(true);
    mSaveButton->SetEnabled(true);
    mLoadButton->SetEnabled(false);
    gLayerManager.PushLayer(&mSaveLayer);

    PopulateSaveList();
    mTextInput->SetEnabled(false);
}

void SaveLoadScreen::ShowLoad()
{
    SetActive(true);
    mSaveButton->SetEnabled(false);
    mLoadButton->SetEnabled(true);
    gLayerManager.PushLayer(&mLoadLayer);

    PopulateSaveList();
    mTextInput->SetEnabled(false);
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
}

void SaveLoadScreen::OnUpdate(float deltaTime)
{
    if(gInputManager.IsKeyLeadingEdge(SDL_SCANCODE_UP) && mSaveIndex > 0)
    {
        SetSelectedSaveIndex(mSaveIndex - 1);
    }
    if(gInputManager.IsKeyLeadingEdge(SDL_SCANCODE_DOWN) && mSaveIndex < mSaveEntryCount - 1)
    {
        SetSelectedSaveIndex(mSaveIndex + 1);
    }
    if(gInputManager.IsKeyLeadingEdge(SDL_SCANCODE_ESCAPE))
    {
        OnExitButtonPressed();
    }
    if(gInputManager.IsKeyLeadingEdge(SDL_SCANCODE_RETURN))
    {
        if(mSaveButton->IsEnabled())
        {
            OnSaveButtonPressed();
        }
        else if(mLoadButton->IsEnabled())
        {
            OnLoadButtonPressed();
        }
    }
}

void SaveLoadScreen::PopulateSaveList()
{
    // Hide all existing list entries.
    for(SaveListEntry& entry : mListEntries)
    {
        entry.button->GetOwner()->SetActive(false);
    }

    // Get existing saves from save manager.
    const std::vector<SaveSummary>& saves = gSaveManager.GetSaves();

    // If in save mode, we create one more entry for an "empty" slot.
    int entryCount = saves.size();
    if(mSaveButton->IsEnabled())
    {
        entryCount++;
    }
    mSaveEntryCount = entryCount;

    // Create a list entry for each save (and maybe also empty slot).
    for(int i = 0; i < entryCount; ++i)
    {
        SaveListEntry* entry = nullptr;
        if(i < mListEntries.size())
        {
            // Reuse an existing entry.
            entry = &mListEntries[i];
        }
        else
        {
            // Create a new entry.
            mListEntries.emplace_back();
            entry = &mListEntries.back();

            // Parent container for the three labels.
            UIButton* button = UIUtil::NewUIActorWithWidget<UIButton>(mListActor);
            button->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
            button->GetRectTransform()->SetSizeDelta(kSaveListWidth, 14.0f);
            button->SetPressCallback([this, i](UIButton* button){
                OnEntryButtonPressed(i);
            });
            mListEntries.back().button = button;

            // Name label.
            UILabel* nameLabel = UIUtil::NewUIActorWithWidget<UILabel>(button->GetOwner());
            nameLabel->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
            nameLabel->GetRectTransform()->SetAnchoredPosition(2.0f, -2.0f);
            nameLabel->GetRectTransform()->SetSizeDelta(kLeftColumnWidth, 14.0f);
            nameLabel->SetFont(gAssetManager.LoadFont("F_SSERIF_T8.FON"));
            nameLabel->SetVerticalAlignment(VerticalAlignment::Top);
            mListEntries.back().nameLabel = nameLabel;

            // Day & time label.
            UILabel* dayTimeLabel = UIUtil::NewUIActorWithWidget<UILabel>(button->GetOwner());
            dayTimeLabel->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
            dayTimeLabel->GetRectTransform()->SetAnchoredPosition(kLeftColumnWidth + kColumnSeparatorWidth + 2.0f, -2.0f);
            dayTimeLabel->GetRectTransform()->SetSizeDelta(kMiddleColumnWidth, 14.0f);
            dayTimeLabel->SetFont(gAssetManager.LoadFont("F_SSERIF_T8.FON"));
            dayTimeLabel->SetVerticalAlignment(VerticalAlignment::Top);
            mListEntries.back().dayTimeLabel = dayTimeLabel;

            // Score label.
            UILabel* scoreLabel = UIUtil::NewUIActorWithWidget<UILabel>(button->GetOwner());
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
            entry->nameLabel->SetText(save.saveInfo.userDescription);
            entry->dayTimeLabel->SetText(gGameProgress.GetTimeblockDisplayName(save.saveInfo.timeblock));
            entry->scoreLabel->SetText(StringUtil::Format("%03i / %03i", save.saveInfo.score, save.saveInfo.maxScore));
        }
        else // empty slot
        {
            entry->nameLabel->SetText("[Empty]");
        }

        // Position appropriately in list.
        entry->button->GetRectTransform()->SetAnchoredPosition(0.0f, i * -kRowHeight);

        // Highlight should default to last item in the list. Just setting it each time works too.
        SetSelectedSaveIndex(i);
    }
}

void SaveLoadScreen::SetSelectedSaveIndex(int index)
{
    mSaveIndex = index;
    mHighlight->SetAnchoredPosition(0.0f, index * -kRowHeight);
}

void SaveLoadScreen::ActivateTextInput(int index)
{
    // Show text input over the name label.
    mTextInput->GetRectTransform()->SetAnchoredPosition(0.0f, index * -kRowHeight);
    mTextInput->SetEnabled(true);

    // If this is the "empty" slot at the end of the list, clear the text to enter a full name.
    // Otherwise, the text stays so the player can keep it if they want.
    if(index >= gSaveManager.GetSaves().size())
    {
        mTextInput->Clear();
        mListEntries[index].nameLabel->SetText("");
    }
    else
    {
        // Show the previously entered text in text input.
        mTextInput->SetText(mListEntries[index].nameLabel->GetText());
        mListEntries[index].nameLabel->SetText("");
    }

    // Player must enter text or exit out at this point.
    mTextInput->Focus();
}

void SaveLoadScreen::OnEntryButtonPressed(int index)
{
    // When text input is enabled, pressing entry buttons has no effect anymore.
    if(mTextInput->IsEnabled())
    {
        return;
    }

    // In save mode, if we select the "empty" slot when it was already selected...
    // It means we want to enter a name for the empty slot.
    if(index == mSaveIndex && mSaveButton->IsEnabled())
    {
        ActivateTextInput(index);
    }

    // Highlight selected button index.
    SetSelectedSaveIndex(index);
}

void SaveLoadScreen::OnSaveButtonPressed()
{
    gAudioManager.PlaySFX(gAssetManager.LoadAudio("SIDBUTN-1.WAV"));
    if(mSaveIndex < gSaveManager.GetSaves().size())
    {
        //TODO: show popup asking if we want to overwrite the save.
        if(!mTextInput->IsEnabled())
        {
            ActivateTextInput(mSaveIndex);
        }
        else
        {
            Hide();
            gSaveManager.Save(mTextInput->GetText(), mSaveIndex);
        }
    }
    else // new save in empty slot
    {
        if(!mTextInput->IsEnabled())
        {
            ActivateTextInput(mSaveIndex);
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
    Hide();
    gSaveManager.Load(gSaveManager.GetSaves()[mSaveIndex].filePath);
}

void SaveLoadScreen::OnExitButtonPressed()
{
    gAudioManager.PlaySFX(gAssetManager.LoadAudio("SIDBUTN-1.WAV"));
    Hide();
}