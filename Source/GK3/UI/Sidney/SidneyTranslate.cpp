#include "SidneyTranslate.h"

#include "Actor.h"
#include "SidneyFiles.h"
#include "SidneyPopup.h"
#include "SidneyUtil.h"
#include "UIImage.h"
#include "UINineSlice.h"
#include "UIUtil.h"

void SidneyTranslate::Init(Actor* parent, SidneyFiles* sidneyFiles)
{
    mSidneyFiles = sidneyFiles;

    // Add background. This will also be the root for this screen.
    mRoot = SidneyUtil::CreateBackground(parent);
    mRoot->SetName("Translate");

    // Add main menu button.
    SidneyUtil::CreateMainMenuButton(mRoot, [this](){
        Hide();
    });

    // Add menu bar.
    mMenuBar.Init(mRoot, SidneyUtil::GetTranslateLocalizer().GetText("ScreenName"), 140.0f);
    mMenuBar.SetFirstDropdownPosition(24.0f);
    mMenuBar.SetDropdownSpacing(26.0f);

    // "Open" dropdown.
    mMenuBar.AddDropdown(SidneyUtil::GetTranslateLocalizer().GetText("MenuName"));
    {
        // "Open File" choice.
        mMenuBar.AddDropdownChoice(SidneyUtil::GetTranslateLocalizer().GetText("MenuItem1"), [this](){

            // Show the file selector.
            mSidneyFiles->Show([this](SidneyFile* selectedFile){
                mTranslateFileId = selectedFile->id;

                // The translate screen can only open text files.
                if(selectedFile->type != SidneyFileType::Text)
                {
                    mPopup->ResetToDefaults();
                    mPopup->SetTextAlignment(HorizontalAlignment::Center);
                    mPopup->SetText(SidneyUtil::GetTranslateLocalizer().GetText("NotTranslatable"));
                    mPopup->ShowOneButton();
                }
                else
                {
                    printf("Translate this!\n");
                }
            });
        });
    }
    
    // Create box/window for main ID making area.
    Actor* windowActor = new Actor(TransformType::RectTransform);
    windowActor->GetTransform()->SetParent(mRoot->GetTransform());
    {
        UINineSlice* border = windowActor->AddComponent<UINineSlice>(SidneyUtil::GetGrayBoxParams(SidneyUtil::TransBgColor));
        border->GetRectTransform()->SetSizeDelta(526.0f, 340.0f);

        // Add divider line for box header.
        UIImage* headerDividerImage = UIUtil::NewUIActorWithWidget<UIImage>(windowActor);
        headerDividerImage->SetTexture(gAssetManager.LoadTexture("S_BOX_TOP.BMP"), true);
        headerDividerImage->GetRectTransform()->SetAnchor(AnchorPreset::TopStretch);
        headerDividerImage->GetRectTransform()->SetAnchoredPosition(0.0f, -17.0f);
        headerDividerImage->GetRectTransform()->SetSizeDeltaX(0.0f);

        // Add another one for the bottom section.
        UIImage* bottomDividerImage = UIUtil::NewUIActorWithWidget<UIImage>(windowActor);
        bottomDividerImage->SetTexture(gAssetManager.LoadTexture("S_BOX_TOP.BMP"), true);
        bottomDividerImage->GetRectTransform()->SetAnchor(AnchorPreset::TopStretch);
        bottomDividerImage->GetRectTransform()->SetAnchoredPosition(0.0f, -256.0f);
        bottomDividerImage->GetRectTransform()->SetSizeDeltaX(0.0f);
    }

    // Analyze message box.
    mPopup = new SidneyPopup(mRoot);

    // Hide by default.
    Hide();
}

void SidneyTranslate::Show()
{
    mRoot->SetActive(true);
}

void SidneyTranslate::Hide()
{
    mRoot->SetActive(false);
}

void SidneyTranslate::OnUpdate(float deltaTime)
{
    if(!mRoot->IsActive()) { return; }
    mMenuBar.Update();
}