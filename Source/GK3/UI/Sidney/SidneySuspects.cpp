#include "SidneySuspects.h"

#include "ActionManager.h"
#include "Actor.h"
#include "AssetManager.h"
#include "Font.h"
#include "GameProgress.h"
#include "Scene.h"
#include "SidneyFiles.h"
#include "SidneyPopup.h"
#include "SidneyUtil.h"
#include "Texture.h"
#include "UIButton.h"
#include "UIImage.h"
#include "UILabel.h"
#include "UINineSlice.h"
#include "UITextInput.h"
#include "UIUtil.h"

void SidneySuspects::Init(Actor* parent, SidneyFiles* sidneyFiles)
{
    mFiles = sidneyFiles;

    // Add background. This will also be the root for this screen.
    mRoot = SidneyUtil::CreateBackground(parent);
    mRoot->SetName("Suspects");

    // Add main menu button.
    SidneyUtil::CreateMainMenuButton(mRoot, [&](){
        Hide();
    });

    // Add menu bar.
    mMenuBar.Init(mRoot, SidneyUtil::GetSuspectsLocalizer().GetText("ScreenName"), 124.0f);

    // File menu.
    mMenuBar.AddDropdown(SidneyUtil::GetSuspectsLocalizer().GetText("Menu1Name"));
    mMenuBar.AddDropdownChoice(SidneyUtil::GetSuspectsLocalizer().GetText("MenuItem1"), [this](){
        // Show a file chooser.
        mFiles->Show([this](SidneyFile* file){
            ShowFile(file);
        });

        // The file chooser on this screen shows in a slightly different position.
        // This is kind of a HACK to enable that - could maybe improve the File Window API to make this better...
        Actor* window = mFiles->GetShowingFileWindow();
        if(window != nullptr)
        {
            window->GetComponent<RectTransform>()->SetAnchoredPosition(13.0f, -92.0f);
        }
    });

    // Suspects menu.
    mMenuBar.AddDropdown(SidneyUtil::GetSuspectsLocalizer().GetText("Menu2Name"));
    // Suspects are added in the show function below!

    // Link menu.
    mMenuBar.AddDropdown(SidneyUtil::GetSuspectsLocalizer().GetText("Menu3Name"));
    mMenuBar.AddDropdownChoice(SidneyUtil::GetSuspectsLocalizer().GetText("Menu3Item1"), [this](){
        OnLinkToSuspectPressed();
    });
    mMenuBar.AddDropdownChoice(SidneyUtil::GetSuspectsLocalizer().GetText("Menu3Item2"), [this](){
        OnUnlinkToSuspectPressed();
    });
    //TODO: separator
    mMenuBar.AddDropdownChoice(SidneyUtil::GetSuspectsLocalizer().GetText("Menu3Item4"), [this](){
        OnMatchAnalysisPressed();
    });

    // Create window with suspect data.
    {
        // Window border.
        UINineSlice* windowBorder = UIUtil::NewUIActorWithWidget<UINineSlice>(mRoot, SidneyUtil::GetGrayBoxParams(SidneyUtil::TransBgColor));
        windowBorder->GetRectTransform()->SetAnchor(AnchorPreset::BottomLeft);
        windowBorder->GetRectTransform()->SetAnchoredPosition(181.0f, 63.0f);
        windowBorder->GetRectTransform()->SetSizeDelta(432.0f, 325.0f);
        mSuspectDataWindow = windowBorder->GetOwner();
        mSuspectDataWindow->SetActive(false);

        // Add header divider border.
        UIImage* headerDividerImage = UIUtil::NewUIActorWithWidget<UIImage>(windowBorder->GetOwner());
        headerDividerImage->SetTexture(gAssetManager.LoadTexture("S_BOX_TOP.BMP"), true);
        headerDividerImage->GetRectTransform()->SetAnchor(AnchorPreset::TopStretch);
        headerDividerImage->GetRectTransform()->SetAnchoredPosition(0.0f, -76.0f);
        headerDividerImage->GetRectTransform()->SetSizeDeltaX(0.0f);

        // Add fingerprint area.
        {
            UINineSlice* fingerprintArea = UIUtil::NewUIActorWithWidget<UINineSlice>(windowBorder->GetOwner(), SidneyUtil::GetGrayBoxParams(SidneyUtil::TransBgColor));
            fingerprintArea->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
            fingerprintArea->GetRectTransform()->SetSizeDelta(70.0f, 77.0f);

            // Add image for fingerprint.
            mFingerprintImage = UIUtil::NewUIActorWithWidget<UIImage>(fingerprintArea->GetOwner());
            mFingerprintImage->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
            mFingerprintImage->GetRectTransform()->SetAnchoredPosition(14.0f, -13.0f);
            mFingerprintImage->GetRectTransform()->SetSizeDelta(41.0f, 51.0f);
            mFingerprintImage->SetEnabled(false);
        }

        // In the header, create all the data fields.
        Font* font = gAssetManager.LoadFont("SID_TEXT_14.FON");
        {
            // Name
            {
                UILabel* nameLabel = UIUtil::NewUIActorWithWidget<UILabel>(mSuspectDataWindow);
                nameLabel->SetFont(font);
                nameLabel->SetText(SidneyUtil::GetSuspectsLocalizer().GetText("Name"));
                nameLabel->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
                nameLabel->GetRectTransform()->SetAnchoredPosition(78.0f, -12.0f);
                nameLabel->GetRectTransform()->SetSizeDelta(nameLabel->GetTextWidth(), font->GetGlyphHeight());

                UINineSlice* nameBorder = UIUtil::NewUIActorWithWidget<UINineSlice>(mSuspectDataWindow, SidneyUtil::GetGrayBoxParams(Color32::Black));
                nameBorder->GetRectTransform()->SetSizeDelta(245.0f, 18.0f);
                nameBorder->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
                nameBorder->GetRectTransform()->SetAnchoredPosition(178.0f, -10.0f);

                mNameLabel = UIUtil::NewUIActorWithWidget<UILabel>(nameBorder->GetOwner());
                mNameLabel->SetFont(font);
                mNameLabel->GetRectTransform()->SetAnchor(AnchorPreset::CenterStretch);
                mNameLabel->GetRectTransform()->SetSizeDelta(-8.0f, -3.0f);
            }

            // Nationality
            {
                UILabel* nationalityLabel = UIUtil::NewUIActorWithWidget<UILabel>(mSuspectDataWindow);
                nationalityLabel->SetFont(font);
                nationalityLabel->SetText(SidneyUtil::GetSuspectsLocalizer().GetText("Nationality"));
                nationalityLabel->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
                nationalityLabel->GetRectTransform()->SetAnchoredPosition(78.0f, -32.0f);
                nationalityLabel->GetRectTransform()->SetSizeDelta(nationalityLabel->GetTextWidth(), font->GetGlyphHeight());

                UINineSlice* nationalityBorder = UIUtil::NewUIActorWithWidget<UINineSlice>(mSuspectDataWindow, SidneyUtil::GetGrayBoxParams(Color32::Black));
                nationalityBorder->GetRectTransform()->SetSizeDelta(245.0f, 18.0f);
                nationalityBorder->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
                nationalityBorder->GetRectTransform()->SetAnchoredPosition(178.0f, -30.0f);

                mNationalityLabel = UIUtil::NewUIActorWithWidget<UILabel>(nationalityBorder->GetOwner());
                mNationalityLabel->SetFont(font);
                mNationalityLabel->GetRectTransform()->SetAnchor(AnchorPreset::CenterStretch);
                mNationalityLabel->GetRectTransform()->SetSizeDelta(-8.0f, -3.0f);
            }

            // Vehicle ID
            {
                UILabel* vehicleLabel = UIUtil::NewUIActorWithWidget<UILabel>(mSuspectDataWindow);
                vehicleLabel->SetFont(font);
                vehicleLabel->SetText(SidneyUtil::GetSuspectsLocalizer().GetText("Vehicle"));
                vehicleLabel->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
                vehicleLabel->GetRectTransform()->SetAnchoredPosition(78.0f, -52.0f);
                vehicleLabel->GetRectTransform()->SetSizeDelta(vehicleLabel->GetTextWidth(), font->GetGlyphHeight());

                UINineSlice* vehicleBorder = UIUtil::NewUIActorWithWidget<UINineSlice>(mSuspectDataWindow, SidneyUtil::GetGrayBoxParams(Color32::Black));
                vehicleBorder->GetRectTransform()->SetSizeDelta(245.0f, 18.0f);
                vehicleBorder->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
                vehicleBorder->GetRectTransform()->SetAnchoredPosition(178.0f, -50.0f);

                mVehicleLabel = UIUtil::NewUIActorWithWidget<UILabel>(vehicleBorder->GetOwner());
                mVehicleLabel->SetFont(font);
                mVehicleLabel->GetRectTransform()->SetAnchor(AnchorPreset::CenterStretch);
                mVehicleLabel->GetRectTransform()->SetSizeDelta(-8.0f, -3.0f);
            }
        }

        // Notes section.
        {
            // Add notes header.
            UINineSlice* notesHeaderBorder = UIUtil::NewUIActorWithWidget<UINineSlice>(windowBorder->GetOwner(), SidneyUtil::GetGrayBoxParams(SidneyUtil::TransBgColor));
            notesHeaderBorder->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
            notesHeaderBorder->GetRectTransform()->SetAnchoredPosition(0.0f, -76.0f);
            notesHeaderBorder->GetRectTransform()->SetSizeDelta(57.0f, 18.0f);

            // Add notes label.
            UILabel* notesHeaderLabel = UIUtil::NewUIActorWithWidget<UILabel>(notesHeaderBorder->GetOwner());
            notesHeaderLabel->SetFont(font);
            notesHeaderLabel->SetText(SidneyUtil::GetSuspectsLocalizer().GetText("Notes"));
            notesHeaderLabel->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
            notesHeaderLabel->GetRectTransform()->SetAnchoredPosition(8.0f, -5.0f);
            notesHeaderLabel->GetRectTransform()->SetSizeDelta(48.0f, 12.0f);

            // Add the notes text input.
            mNotesInput = UIUtil::NewUIActorWithWidget<UITextInput>(windowBorder->GetOwner());
            mNotesInput->SetMaxLength(500);
            mNotesInput->SetFont(font);
            mNotesInput->SetHorizonalAlignment(HorizontalAlignment::Left);
            mNotesInput->SetVerticalAlignment(VerticalAlignment::Top);
            mNotesInput->SetHorizontalOverflow(HorizontalOverflow::Wrap);
            mNotesInput->SetVerticalOverflow(VerticalOverflow::Truncate);
            mNotesInput->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
            mNotesInput->GetRectTransform()->SetAnchoredPosition(9.0f, -102.0f);
            mNotesInput->GetRectTransform()->SetSizeDelta(406.0f, 134.0f);

            // Create text input caret.
            UIImage* caretImage = UIUtil::NewUIActorWithWidget<UIImage>(mNotesInput->GetOwner());
            caretImage->SetTexture(&Texture::White);
            caretImage->SetColor(Color32(198, 170, 41));
            caretImage->GetRectTransform()->SetAnchor(AnchorPreset::BottomLeft);
            caretImage->GetRectTransform()->SetPivot(0.0f, 0.0f);
            caretImage->GetRectTransform()->SetSizeDelta(2.0f, font->GetGlyphHeight());
            mNotesInput->SetCaret(caretImage);
            mNotesInput->SetCaretBlinkInterval(0.5f);
        }

        // Footer area
        {
            // Add footer divider border.
            UIImage* footerDividerImage = UIUtil::NewUIActorWithWidget<UIImage>(windowBorder->GetOwner());
            footerDividerImage->SetTexture(gAssetManager.LoadTexture("S_BOX_TOP.BMP"), true);
            footerDividerImage->GetRectTransform()->SetAnchor(AnchorPreset::BottomStretch);
            footerDividerImage->GetRectTransform()->SetAnchoredPosition(0.0f, 74.0f);
            footerDividerImage->GetRectTransform()->SetSizeDeltaX(0.0f);

            // Add linked items.
            {
                for(int i = 0; i < 6; ++i)
                {
                    // Linked item border.
                    UINineSlice* itemBorder = UIUtil::NewUIActorWithWidget<UINineSlice>(windowBorder->GetOwner(), SidneyUtil::GetGrayBoxParams(SidneyUtil::TransBgColor));
                    itemBorder->GetRectTransform()->SetAnchor(AnchorPreset::BottomLeft);
                    itemBorder->GetRectTransform()->SetAnchoredPosition(i * 70.0f, 0.0f);
                    itemBorder->GetRectTransform()->SetSizeDelta(70.0f, 74.0f);

                    // Header border.
                    UINineSlice* headerBorder = UIUtil::NewUIActorWithWidget<UINineSlice>(itemBorder->GetOwner(), SidneyUtil::GetGrayBoxParams(SidneyUtil::TransBgColor));
                    headerBorder->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
                    headerBorder->GetRectTransform()->SetAnchoredPosition(0.0f, 0.0f);
                    headerBorder->GetRectTransform()->SetSizeDelta(70.0f, 18.0f);

                    // Header label.
                    UILabel* itemLabel = UIUtil::NewUIActorWithWidget<UILabel>(itemBorder->GetOwner());
                    itemLabel->SetFont(font);
                    itemLabel->SetText("Mose_Pr");
                    itemLabel->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
                    itemLabel->GetRectTransform()->SetAnchoredPosition(4.0f, -3.0f);
                    itemLabel->GetRectTransform()->SetSizeDelta(49.0f, 14.0f);

                    // Image.
                    UIImage* itemImage = UIUtil::NewUIActorWithWidget<UIImage>(itemBorder->GetOwner());
                    itemImage->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
                    itemImage->GetRectTransform()->SetAnchoredPosition(19.0f, -29.0f);
                    itemImage->GetRectTransform()->SetSizeDelta(32.0f, 30.0f);

                    // The whole item has a button on it so it can be clicked/selected.
                    UIButton* itemButton = UIUtil::NewUIActorWithWidget<UIButton>(itemBorder->GetOwner());
                    itemButton->GetRectTransform()->SetAnchor(AnchorPreset::CenterStretch);
                    itemButton->GetRectTransform()->SetSizeDelta(0.0f, 0.0f);
                    itemButton->SetPressCallback([this, i](UIButton* button){
                        OnLinkedEvidenceSelected(i);
                    });

                    // Save UI elements.
                    mLinkedEvidenceItems[i].root = itemBorder->GetOwner();
                    mLinkedEvidenceItems[i].border = headerBorder;
                    mLinkedEvidenceItems[i].label = itemLabel;
                    mLinkedEvidenceItems[i].image = itemImage;

                    // Hide by default.
                    mLinkedEvidenceItems[i].root->SetActive(false);
                }   
            }
        }
    }

    // Create file preview window.
    {
        FilePreviewWindow win = SidneyUtil::CreateFilePreviewWindow(mRoot);
        mFilePreviewWindow = win.root;
        mFilePreviewHeaderLabel = win.header;
        mFilePreviewImage = win.image;

        mFilePreviewWindow->GetComponent<RectTransform>()->SetAnchor(AnchorPreset::TopLeft);
        mFilePreviewWindow->GetComponent<RectTransform>()->SetAnchoredPosition(13.0f, -92.0f);
        
        mFilePreviewWindow->SetActive(false);
    }

    // Create popup.
    mPopup = new SidneyPopup(mRoot);

    // Populate the suspect data.
    // We can do this in an unusual way: rather than having a dedicated "SUSPECTS.TXT" or whatever, the data is in the localizer.
    // So, we can just iterate and retrieve from there.
    for(int i = 1; i <= 10; ++i)
    {
        mSuspectInfos.emplace_back();
        mSuspectInfos.back().name = SidneyUtil::GetSuspectsLocalizer().GetText("Name" + std::to_string(i));
        mSuspectInfos.back().nationality = SidneyUtil::GetSuspectsLocalizer().GetText("Nationality" + std::to_string(i));
        mSuspectInfos.back().vehicleId = SidneyUtil::GetSuspectsLocalizer().GetText("VehicleID" + std::to_string(i));
    }

    // Set initial menu choices enabled or disabled.
    RefreshEnabledMenuChoices();

    // Hide by default.
    Hide();
}

void SidneySuspects::Show()
{
    // At the start of the game, the dropdown doesn't show all suspects. You add a few later.
    // First, add the initial bunch.
    if(mMenuBar.GetDropdownChoiceCount(1) == 0)
    {
        for(int i = 0; i < 8; ++i)
        {
            mMenuBar.AddDropdownChoice(1, SidneyUtil::GetSuspectsLocalizer().GetText("Menu2Item" + std::to_string(i + 1)), [this, i](){
                ShowSuspect(i);
            });
        }
    }

    // Show the root.
    mRoot->SetActive(true);
}

void SidneySuspects::Hide()
{
    // Make sure any showing file list is closed.
    mFiles->HideAllFileWindows();

    // Hide the root.
    mRoot->SetActive(false);
}

void SidneySuspects::OnUpdate(float deltaTime)
{
    mMenuBar.Update();
}

void SidneySuspects::ShowSuspect(int index)
{
    // Get the suspect info.
    assert(index >= 0 && index < mSuspectInfos.size());
    SuspectInfo& info = mSuspectInfos[index];

    // Turn on the suspect window.
    mSuspectDataWindow->SetActive(true);

    // Set name and nationality, which are always shown no matter what.
    mNameLabel->SetText(info.name);
    mNationalityLabel->SetText(info.nationality);

    // Set notes, which are also set no matter what.
    mNotesInput->SetText(info.notes);

    // The vehicle ID only shows if the associated file is linked.
    bool showVehicle = mVehicleIdLinkedFiles[index].fileId == -1;
    if(!showVehicle)
    {
        showVehicle = std::find(info.linkedFileIds.begin(), info.linkedFileIds.end(), mVehicleIdLinkedFiles[index].fileId) != info.linkedFileIds.end();
    }
    if(showVehicle)
    {
        mVehicleLabel->SetText(info.vehicleId);
    }
    else
    {
        mVehicleLabel->SetText("");
    }

    // Similarly, fingerprint image only shows if associated file is linked.
    auto fingerprintFileIt = std::find(info.linkedFileIds.begin(), info.linkedFileIds.end(), mFingerprintLinkedFiles[index].fileId);
    if(fingerprintFileIt != info.linkedFileIds.end())
    {
        mFingerprintImage->SetEnabled(true);
        mFingerprintImage->SetTexture(mFiles->GetFile(*fingerprintFileIt)->GetIcon());
    }
    else
    {
        mFingerprintImage->SetEnabled(false);
    }

    // Show linked evidence UI as needed.
    int evidenceItemIndex = 0;
    for(int i = 0; i < info.linkedFileIds.size(); ++i)
    {
        SidneyFile* file = mFiles->GetFile(info.linkedFileIds[i]);
        if(file == nullptr) { continue; }

        // Show this evidence item.
        mLinkedEvidenceItems[evidenceItemIndex].root->SetActive(true);

        // Populate the data.
        mLinkedEvidenceItems[evidenceItemIndex].label->SetText(file->GetDisplayName());
        mLinkedEvidenceItems[evidenceItemIndex].image->SetTexture(file->GetIcon());

        // Set highlight based on whether this item is selected or not.
        if(info.selectedLinkedFileIndex == i)
        {
            mLinkedEvidenceItems[evidenceItemIndex].border->SetTexturesAndColors(SidneyUtil::GetGoldBoxParams(SidneyUtil::TransBgColor), false);
        }
        else
        {
            mLinkedEvidenceItems[evidenceItemIndex].border->SetTexturesAndColors(SidneyUtil::GetGrayBoxParams(SidneyUtil::TransBgColor), false);
        }
        ++evidenceItemIndex;
    }
    for(; evidenceItemIndex < kMaxLinkedEvidence; ++evidenceItemIndex)
    {
        mLinkedEvidenceItems[evidenceItemIndex].root->SetActive(false);
    }
    
    // Save the opened suspect index.
    mOpenedSuspectIndex = index;

    // The notes field should be automatically focused.
    mNotesInput->Focus();

    // Upon opening a suspect, some menu choices may enable.
    RefreshEnabledMenuChoices();

    // If this is the first time accessing suspects, Gabe or Grace may play a VO line.
    if(!gGameProgress.GetFlag("GabeSawSuspects") && !gGameProgress.GetFlag("GraceSawSuspects"))
    {
        // Play VO for Gabe or Grace.
        if(StringUtil::EqualsIgnoreCase(Scene::GetEgoName(), "Gabriel"))
        {
            gActionManager.ExecuteSheepAction("wait StartDialogue(\"02OC52ZPF2\", 1)");
        }
        else
        {
            gActionManager.ExecuteSheepAction("wait StartDialogue(\"02OC52ZPF1\", 1)");
        }

        // It appears that if you trigger this VO for a character, you don't get it for the other character.
        //TODO: Setting both here, but need to check what OG does.
        gGameProgress.SetFlag("GabeSawSuspects");
        gGameProgress.SetFlag("GraceSawSuspects");
    }
}

void SidneySuspects::ShowFile(SidneyFile* file)
{
    if(file != nullptr)
    {
        mOpenedFileId = file->id;
        mFilePreviewWindow->SetActive(true);
        mFilePreviewHeaderLabel->SetText(file->GetDisplayName());
        mFilePreviewImage->SetTexture(file->GetIcon());
    }
    else
    {
        mOpenedFileId = -1;
        mFilePreviewWindow->SetActive(false);
    }

    // Upon opening a file, some menu choices may enable.
    RefreshEnabledMenuChoices();
}

void SidneySuspects::RefreshEnabledMenuChoices()
{
    // The third menu's options are only available if both a suspect and a file are open.
    bool suspectOpen = mOpenedSuspectIndex >= 0;
    bool fileOpen = mOpenedFileId >= 0;
    mMenuBar.SetDropdownChoiceEnabled(2, 0, suspectOpen && fileOpen);
    mMenuBar.SetDropdownChoiceEnabled(2, 1, suspectOpen && fileOpen);

    // The final option is only available under very specific circumstances (TODO).
    mMenuBar.SetDropdownChoiceEnabled(2, 2, false);
}

void SidneySuspects::OnLinkToSuspectPressed()
{
    // Must have a suspect and a file opened.
    if(mOpenedSuspectIndex < 0 || mOpenedFileId < 0) { return; }
    SuspectInfo& info = mSuspectInfos[mOpenedSuspectIndex];

    // You can only link up to six files per suspect.
    if(info.linkedFileIds.size() >= 6)
    {
        // Show an error popup.
        mPopup->ResetToDefaults();
        mPopup->SetTextAlignment(HorizontalAlignment::Center);
        mPopup->SetText(SidneyUtil::GetSuspectsLocalizer().GetText("MaxLinks"));
        mPopup->ShowOneButton();
        return;
    }

    // See if this file is one of the important ones for this suspect.
    // These are always allowed to be linked.
    bool alreadyLinked = false;
    bool isThisSuspectsVehicleFile = (mOpenedFileId == mVehicleIdLinkedFiles[mOpenedSuspectIndex].fileId);
    bool isThisSuspectsFingerprintFile = (mOpenedFileId == mFingerprintLinkedFiles[mOpenedSuspectIndex].fileId);
    if(!isThisSuspectsVehicleFile && !isThisSuspectsFingerprintFile)
    {
        // If it's NOT one of the important files, you can still link it...but only if not linked to anyone else.
        for(auto& suspect : mSuspectInfos)
        {
            for(auto& linkedFileId : suspect.linkedFileIds)
            {
                if(linkedFileId == mOpenedFileId)
                {
                    alreadyLinked = true;
                    break;
                }
            }
            if(alreadyLinked) { break; }
        }   
    }
    else
    {
        // One edge case: if it is one of our important files, but its already been linked to us, we still show the error popup.
        alreadyLinked = std::find(info.linkedFileIds.begin(), info.linkedFileIds.end(), mOpenedFileId) != info.linkedFileIds.end();
    }

    // If already linked, we show an error and don't move forward.
    if(alreadyLinked)
    {
        mPopup->ResetToDefaults();
        mPopup->SetTextAlignment(HorizontalAlignment::Center);
        mPopup->SetText(SidneyUtil::GetSuspectsLocalizer().GetText("AlreadyLinked"));
        mPopup->ShowOneButton();
        return;
    }
    
    // Link the file.
    info.linkedFileIds.push_back(mOpenedFileId);

    // Set selected index to the last one in the list.
    info.selectedLinkedFileIndex = info.linkedFileIds.size() - 1;

    // Change score if a score name is specified.
    if(isThisSuspectsVehicleFile)
    {
        if(!mVehicleIdLinkedFiles[mOpenedSuspectIndex].scoreName.empty())
        {
            gGameProgress.ChangeScore(mVehicleIdLinkedFiles[mOpenedSuspectIndex].scoreName);
        }
        if(!mVehicleIdLinkedFiles[mOpenedSuspectIndex].flag.empty())
        {
            gGameProgress.SetFlag(mVehicleIdLinkedFiles[mOpenedSuspectIndex].flag);
        }
    }
    if(isThisSuspectsFingerprintFile)
    {
        if(!mFingerprintLinkedFiles[mOpenedSuspectIndex].scoreName.empty())
        {
            gGameProgress.ChangeScore(mFingerprintLinkedFiles[mOpenedSuspectIndex].scoreName);
        }
        if(!mFingerprintLinkedFiles[mOpenedSuspectIndex].flag.empty())
        {
            gGameProgress.SetFlag(mFingerprintLinkedFiles[mOpenedSuspectIndex].flag);
        }
    }

    // Re-show this suspect to refresh the UI.
    ShowSuspect(mOpenedSuspectIndex);
}

void SidneySuspects::OnUnlinkToSuspectPressed()
{
    // Must have a suspect opened.
    if(mOpenedSuspectIndex < 0) { return; }
    SuspectInfo& info = mSuspectInfos[mOpenedSuspectIndex];

    // Which file do we want to unlink? Depends on which is selected (NOT which is opened).
    int selectedFileId = info.linkedFileIds[info.selectedLinkedFileIndex];

    // If this is one of the important files for the suspect, you actually aren't allowed to unlink it.
    bool isThisSuspectsVehicleFile = (selectedFileId == mVehicleIdLinkedFiles[mOpenedSuspectIndex].fileId);
    bool isThisSuspectsFingerprintFile = (selectedFileId == mFingerprintLinkedFiles[mOpenedSuspectIndex].fileId);
    if(isThisSuspectsVehicleFile || isThisSuspectsFingerprintFile)
    {
        // Gabe or grace say "I think that's right, I don't want to unlink it."
        if(StringUtil::EqualsIgnoreCase(Scene::GetEgoName(), "Gabriel"))
        {
            gActionManager.ExecuteSheepAction("wait StartDialogue(\"2FL8S27AS1\", 1)");
        }
        else
        {
            gActionManager.ExecuteSheepAction("wait StartDialogue(\"02O9H7Z411\", 1)");
        }
        return;
    }

    // Otherwise, we can unlink it.
    info.linkedFileIds.erase(info.linkedFileIds.begin() + info.selectedLinkedFileIndex);

    // Keep the selected index variable in a valid state.
    if(info.linkedFileIds.empty())
    {
        info.selectedLinkedFileIndex = -1;
    }
    else
    {
        info.selectedLinkedFileIndex = Math::Clamp(info.selectedLinkedFileIndex, 0, info.linkedFileIds.size() - 1);
    }

    // Re-show this suspect to refresh the UI.
    ShowSuspect(mOpenedSuspectIndex);
}

void SidneySuspects::OnMatchAnalysisPressed()
{
    printf("Match Analysis\n");
}

void SidneySuspects::OnLinkedEvidenceSelected(int index)
{
    // Change the selected file index.
    mSuspectInfos[mOpenedSuspectIndex].selectedLinkedFileIndex = index;

    // Re-show the suspect info to change the highlight.
    ShowSuspect(mOpenedSuspectIndex);
}
