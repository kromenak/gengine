#include "SaveLoadScreen.h"

#include "AssetManager.h"
#include "AudioManager.h"
#include "RectTransform.h"
#include "Texture.h"
#include "UIButton.h"
#include "UICanvas.h"
#include "UIImage.h"
#include "UIUtil.h"
#include "Window.h"

SaveLoadScreen::SaveLoadScreen() : Actor(TransformType::RectTransform),
    mSaveLayer("SaveLayer"),
    mLoadLayer("LoadLayer")
{
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
            gAudioManager.PlaySFX(gAssetManager.LoadAudio("SIDBUTN-1.WAV"));
            Hide();
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
            gAudioManager.PlaySFX(gAssetManager.LoadAudio("SIDBUTN-1.WAV"));
            printf("Save\n");
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
            gAudioManager.PlaySFX(gAssetManager.LoadAudio("SIDBUTN-1.WAV"));
            printf("Restore\n");
        });
    }

    // Save/load layers stop scene SFX/VO, but not music or ambient.
    mSaveLayer.OverrideAudioState(true, true, false);
    mLoadLayer.OverrideAudioState(true, true, false);
}

void SaveLoadScreen::ShowSave()
{
    SetActive(true);
    mSaveButton->SetEnabled(true);
    mLoadButton->SetEnabled(false);
    gLayerManager.PushLayer(&mSaveLayer);
}

void SaveLoadScreen::ShowLoad()
{
    SetActive(true);
    mSaveButton->SetEnabled(false);
    mLoadButton->SetEnabled(true);
    gLayerManager.PushLayer(&mLoadLayer);
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