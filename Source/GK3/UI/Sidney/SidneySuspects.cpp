#include "SidneySuspects.h"

#include "ActionManager.h"
#include "Actor.h"
#include "AssetManager.h"
#include "Font.h"
#include "GameProgress.h"
#include "GK3UI.h"
#include "Scene.h"
#include "SidneyButton.h"
#include "SidneyFiles.h"
#include "SidneyPopup.h"
#include "SidneyUtil.h"
#include "Texture.h"
#include "Timers.h"
#include "UIButton.h"
#include "UIImage.h"
#include "UILabel.h"
#include "UINineSlice.h"
#include "UITextInput.h"
#include "UIUtil.h"
#include "UIVideoImage.h"

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
    mMenuBar.Init(mRoot, SidneyUtil::GetSuspectsLocalizer().GetText("ScreenName"));

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
    mMenuBar.AddDropdownChoiceSeparator();
    mMenuBar.AddDropdownChoice(SidneyUtil::GetSuspectsLocalizer().GetText("Menu3Item4"), [this](){
        OnMatchAnalysisPressed();
    });

    // Create window with suspect data.
    Font* font = gAssetManager.LoadFont("SID_TEXT_14.FON");
    {
        // Window border.
        UINineSlice* windowBorder = UI::CreateWidgetActor<UINineSlice>("Window", mRoot, SidneyUtil::GetGrayBoxParams(SidneyUtil::TransBgColor));
        windowBorder->GetRectTransform()->SetAnchor(AnchorPreset::BottomLeft);
        windowBorder->GetRectTransform()->SetAnchoredPosition(181.0f, 63.0f);
        windowBorder->GetRectTransform()->SetSizeDelta(432.0f, 325.0f);
        mSuspectDataWindow = windowBorder->GetOwner();
        mSuspectDataWindow->SetActive(false);

        // Add header divider border.
        UIImage* headerDividerImage = UI::CreateWidgetActor<UIImage>("Divider", windowBorder);
        headerDividerImage->SetTexture(gAssetManager.LoadTexture("S_BOX_TOP.BMP"), true);
        headerDividerImage->GetRectTransform()->SetAnchor(AnchorPreset::TopStretch);
        headerDividerImage->GetRectTransform()->SetAnchoredPosition(0.0f, -76.0f);
        headerDividerImage->GetRectTransform()->SetSizeDeltaX(0.0f);

        // Add fingerprint area.
        {
            UINineSlice* fingerprintArea = UI::CreateWidgetActor<UINineSlice>("FingerprintBox", windowBorder, SidneyUtil::GetGrayBoxParams(SidneyUtil::TransBgColor));
            fingerprintArea->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
            fingerprintArea->GetRectTransform()->SetSizeDelta(70.0f, 77.0f);

            // Add image for fingerprint.
            mFingerprintImage = UI::CreateWidgetActor<UIImage>("Fingerprint", fingerprintArea);
            mFingerprintImage->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
            mFingerprintImage->GetRectTransform()->SetAnchoredPosition(14.0f, -13.0f);
            mFingerprintImage->GetRectTransform()->SetSizeDelta(41.0f, 51.0f);
            mFingerprintImage->SetEnabled(false);
        }

        // In the header, create all the data fields.
        {
            // Name
            {
                UILabel* nameLabel = UI::CreateWidgetActor<UILabel>("NameLabel", mSuspectDataWindow);
                nameLabel->SetFont(font);
                nameLabel->SetText(SidneyUtil::GetSuspectsLocalizer().GetText("Name"));
                nameLabel->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
                nameLabel->GetRectTransform()->SetAnchoredPosition(78.0f, -12.0f);
                nameLabel->GetRectTransform()->SetSizeDelta(nameLabel->GetTextWidth(), font->GetGlyphHeight());

                UINineSlice* nameBorder = UI::CreateWidgetActor<UINineSlice>("NameBox", mSuspectDataWindow, SidneyUtil::GetGrayBoxParams(Color32::Black));
                nameBorder->GetRectTransform()->SetSizeDelta(245.0f, 18.0f);
                nameBorder->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
                nameBorder->GetRectTransform()->SetAnchoredPosition(178.0f, -10.0f);

                mNameLabel = UI::CreateWidgetActor<UILabel>("NameValueLabel", nameBorder->GetOwner());
                mNameLabel->SetFont(font);
                mNameLabel->GetRectTransform()->SetAnchor(AnchorPreset::CenterStretch);
                mNameLabel->GetRectTransform()->SetSizeDelta(-8.0f, -3.0f);
            }

            // Nationality
            {
                UILabel* nationalityLabel = UI::CreateWidgetActor<UILabel>("NationalityLabel", mSuspectDataWindow);
                nationalityLabel->SetFont(font);
                nationalityLabel->SetText(SidneyUtil::GetSuspectsLocalizer().GetText("Nationality"));
                nationalityLabel->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
                nationalityLabel->GetRectTransform()->SetAnchoredPosition(78.0f, -32.0f);
                nationalityLabel->GetRectTransform()->SetSizeDelta(nationalityLabel->GetTextWidth(), font->GetGlyphHeight());

                UINineSlice* nationalityBorder = UI::CreateWidgetActor<UINineSlice>("NationalityBox", mSuspectDataWindow, SidneyUtil::GetGrayBoxParams(Color32::Black));
                nationalityBorder->GetRectTransform()->SetSizeDelta(245.0f, 18.0f);
                nationalityBorder->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
                nationalityBorder->GetRectTransform()->SetAnchoredPosition(178.0f, -30.0f);

                mNationalityLabel = UI::CreateWidgetActor<UILabel>("NationalityValueLabel", nationalityBorder);
                mNationalityLabel->SetFont(font);
                mNationalityLabel->GetRectTransform()->SetAnchor(AnchorPreset::CenterStretch);
                mNationalityLabel->GetRectTransform()->SetSizeDelta(-8.0f, -3.0f);
            }

            // Vehicle ID
            {
                UILabel* vehicleLabel = UI::CreateWidgetActor<UILabel>("VehicleLabel", mSuspectDataWindow);
                vehicleLabel->SetFont(font);
                vehicleLabel->SetText(SidneyUtil::GetSuspectsLocalizer().GetText("Vehicle"));
                vehicleLabel->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
                vehicleLabel->GetRectTransform()->SetAnchoredPosition(78.0f, -52.0f);
                vehicleLabel->GetRectTransform()->SetSizeDelta(vehicleLabel->GetTextWidth(), font->GetGlyphHeight());

                UINineSlice* vehicleBorder = UI::CreateWidgetActor<UINineSlice>("VehicleBox", mSuspectDataWindow, SidneyUtil::GetGrayBoxParams(Color32::Black));
                vehicleBorder->GetRectTransform()->SetSizeDelta(245.0f, 18.0f);
                vehicleBorder->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
                vehicleBorder->GetRectTransform()->SetAnchoredPosition(178.0f, -50.0f);

                mVehicleLabel = UI::CreateWidgetActor<UILabel>("VehicleValueLabel", vehicleBorder);
                mVehicleLabel->SetFont(font);
                mVehicleLabel->GetRectTransform()->SetAnchor(AnchorPreset::CenterStretch);
                mVehicleLabel->GetRectTransform()->SetSizeDelta(-8.0f, -3.0f);
            }
        }

        // Notes section.
        {
            // Add notes border.
            UINineSlice* notesHeaderBorder = UI::CreateWidgetActor<UINineSlice>("NotesBox", windowBorder, SidneyUtil::GetGrayBoxParams(SidneyUtil::TransBgColor));
            notesHeaderBorder->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
            notesHeaderBorder->GetRectTransform()->SetAnchoredPosition(0.0f, -76.0f);
            notesHeaderBorder->GetRectTransform()->SetSizeDelta(57.0f, 18.0f);

            // Add notes label.
            UILabel* notesHeaderLabel = UI::CreateWidgetActor<UILabel>("NotesHeaderLabel", notesHeaderBorder);
            notesHeaderLabel->SetFont(font);
            notesHeaderLabel->SetText(SidneyUtil::GetSuspectsLocalizer().GetText("Notes"));
            notesHeaderLabel->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
            notesHeaderLabel->GetRectTransform()->SetAnchoredPosition(8.0f, -5.0f);
            notesHeaderLabel->GetRectTransform()->SetSizeDelta(48.0f, 12.0f);

            // Add the notes text input.
            mNotesInput = UI::CreateWidgetActor<UITextInput>("NotesInput", windowBorder);
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
            UIImage* caretImage = UI::CreateWidgetActor<UIImage>("Caret", mNotesInput);
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
            UIImage* footerDividerImage = UI::CreateWidgetActor<UIImage>("DividerBottom", windowBorder);
            footerDividerImage->SetTexture(gAssetManager.LoadTexture("S_BOX_TOP.BMP"), true);
            footerDividerImage->GetRectTransform()->SetAnchor(AnchorPreset::BottomStretch);
            footerDividerImage->GetRectTransform()->SetAnchoredPosition(0.0f, 74.0f);
            footerDividerImage->GetRectTransform()->SetSizeDeltaX(0.0f);

            // Add linked items.
            {
                for(int i = 0; i < 6; ++i)
                {
                    // Linked item border.
                    UINineSlice* itemBorder = UI::CreateWidgetActor<UINineSlice>("LinkedItem" + std::to_string(i), windowBorder, SidneyUtil::GetGrayBoxParams(SidneyUtil::TransBgColor));
                    itemBorder->GetRectTransform()->SetAnchor(AnchorPreset::BottomLeft);
                    itemBorder->GetRectTransform()->SetAnchoredPosition(i * 70.0f, 0.0f);
                    itemBorder->GetRectTransform()->SetSizeDelta(70.0f, 75.0f);

                    // Header border.
                    UINineSlice* headerBorder = UI::CreateWidgetActor<UINineSlice>("HeaderBox", itemBorder, SidneyUtil::GetGrayBoxParams(SidneyUtil::TransBgColor));
                    headerBorder->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
                    headerBorder->GetRectTransform()->SetSizeDelta(70.0f, 18.0f);

                    // Header label.
                    UILabel* itemLabel = UI::CreateWidgetActor<UILabel>("Header", itemBorder);
                    itemLabel->SetFont(font);
                    itemLabel->SetText("Mose_Pr");
                    itemLabel->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
                    itemLabel->GetRectTransform()->SetAnchoredPosition(4.0f, -3.0f);
                    itemLabel->GetRectTransform()->SetSizeDelta(49.0f, 14.0f);

                    // Image.
                    UIImage* itemImage = UI::CreateWidgetActor<UIImage>("Image", itemBorder);
                    itemImage->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
                    itemImage->GetRectTransform()->SetAnchoredPosition(19.0f, -29.0f);
                    itemImage->GetRectTransform()->SetSizeDelta(32.0f, 30.0f);

                    // The whole item has a button on it so it can be clicked/selected.
                    UIButton* itemButton = UI::CreateWidgetActor<UIButton>("Button", itemBorder);
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

    // Create match analysis window.
    {
        // Window border.
        UINineSlice* windowBorder = UI::CreateWidgetActor<UINineSlice>("MatchAnalysisWindow", mRoot, SidneyUtil::GetGrayBoxParams(SidneyUtil::TransBgColor));
        windowBorder->GetRectTransform()->SetSizeDelta(267.0f, 156.0f);
        windowBorder->GetRectTransform()->SetAnchoredPosition(0.0f, -15.0f);
        mMatchAnalysisWindow = windowBorder->GetOwner();
        mMatchAnalysisWindow->SetActive(false);

        // Add header divider border.
        UIImage* headerDividerImage = UI::CreateWidgetActor<UIImage>("Divider", windowBorder);
        headerDividerImage->SetTexture(gAssetManager.LoadTexture("S_BOX_TOP.BMP"), true);
        headerDividerImage->GetRectTransform()->SetAnchor(AnchorPreset::TopStretch);
        headerDividerImage->GetRectTransform()->SetAnchoredPosition(0.0f, -18.0f);
        headerDividerImage->GetRectTransform()->SetSizeDeltaX(0.0f);

        // Header label.
        UILabel* headerLabel = UI::CreateWidgetActor<UILabel>("HeaderLabel", windowBorder);
        headerLabel->SetFont(font);
        headerLabel->SetText(SidneyUtil::GetSuspectsLocalizer().GetText("MatchTitle"));
        headerLabel->SetHorizonalAlignment(HorizontalAlignment::Center);
        headerLabel->GetRectTransform()->SetAnchor(AnchorPreset::TopStretch);
        headerLabel->GetRectTransform()->SetSizeDelta(0.0f, 18.0f);

        // Fingerprint border and images.
        UINineSlice* fingerprintBorder = UI::CreateWidgetActor<UINineSlice>("FingerprintBox", windowBorder, SidneyUtil::GetGrayBoxParams(SidneyUtil::TransBgColor));
        fingerprintBorder->GetRectTransform()->SetAnchor(AnchorPreset::BottomLeft);
        fingerprintBorder->GetRectTransform()->SetAnchoredPosition(18.0f, 65.0f);
        fingerprintBorder->GetRectTransform()->SetSizeDelta(49.0f, 59.0f);

        mMAFingerprintImage = UI::CreateWidgetActor<UIImage>("FingerprintImage", fingerprintBorder);
        mMAFingerprintImage->GetRectTransform()->SetAnchor(AnchorPreset::BottomLeft);
        mMAFingerprintImage->GetRectTransform()->SetAnchoredPosition(4.0f, 4.0f);
        mMAFingerprintImage->GetRectTransform()->SetSizeDelta(41.0f, 51.0f);

        mMAFingerprintVideoImage = UI::CreateWidgetActor<UIVideoImage>("FingerprintVideoImage", fingerprintBorder);
        mMAFingerprintVideoImage->GetRectTransform()->SetAnchor(AnchorPreset::BottomLeft);
        mMAFingerprintVideoImage->GetRectTransform()->SetAnchoredPosition(4.0f, 4.0f);
        mMAFingerprintVideoImage->GetRectTransform()->SetSizeDelta(41.0f, 51.0f);

        // Current action border and label.
        UINineSlice* actionBorder = UI::CreateWidgetActor<UINineSlice>("ActionBox", windowBorder, SidneyUtil::GetGrayBoxParams(SidneyUtil::TransBgColor));
        actionBorder->GetRectTransform()->SetAnchor(AnchorPreset::BottomLeft);
        actionBorder->GetRectTransform()->SetAnchoredPosition(85.0f, 106.0f);
        actionBorder->GetRectTransform()->SetSizeDelta(170.0f, 18.0f);

        mMAActionLabel = UI::CreateWidgetActor<UILabel>("Label", actionBorder);
        mMAActionLabel->SetFont(gAssetManager.LoadFont("SID_TEXT_14_GRN.FON"));
        mMAActionLabel->SetText("");
        mMAActionLabel->SetHorizonalAlignment(HorizontalAlignment::Center);
        mMAActionLabel->GetRectTransform()->SetAnchor(AnchorPreset::TopStretch);
        mMAActionLabel->GetRectTransform()->SetAnchoredPosition(0.0f, -1.0f);
        mMAActionLabel->GetRectTransform()->SetSizeDelta(0.0f, 16.0f);

        mMASuspectLabel = UI::CreateWidgetActor<UILabel>("SuspectLabel", actionBorder);
        mMASuspectLabel->SetFont(font);
        mMASuspectLabel->SetText("");
        mMASuspectLabel->SetHorizonalAlignment(HorizontalAlignment::Center);
        mMASuspectLabel->GetRectTransform()->SetAnchor(AnchorPreset::TopStretch);
        mMASuspectLabel->GetRectTransform()->SetAnchoredPosition(0.0f, -26.0f);
        mMASuspectLabel->GetRectTransform()->SetSizeDelta(0.0f, 18.0f);

        // Create buttons.
        mMALinkToSuspectButton = new SidneyButton("LinkButton", windowBorder->GetOwner());
        mMALinkToSuspectButton->SetText(SidneyUtil::GetSuspectsLocalizer().GetText("MatchLink"));
        mMALinkToSuspectButton->GetRectTransform()->SetAnchor(AnchorPreset::BottomLeft);
        mMALinkToSuspectButton->GetRectTransform()->SetAnchoredPosition(12.0f, 10.0f);
        mMALinkToSuspectButton->GetRectTransform()->SetSizeDelta(112.0f, 13.0f);
        mMALinkToSuspectButton->SetPressCallback([this](){
            OnLinkToSuspectPressed();
        });

        mMACloseButton = new SidneyButton("CloseButton", windowBorder->GetOwner());
        mMACloseButton->SetText(SidneyUtil::GetSuspectsLocalizer().GetText("MatchClose"));
        mMACloseButton->GetRectTransform()->SetAnchor(AnchorPreset::BottomLeft);
        mMACloseButton->GetRectTransform()->SetAnchoredPosition(144.0f, 10.0f);
        mMACloseButton->GetRectTransform()->SetSizeDelta(112.0f, 13.0f);
        mMACloseButton->SetPressCallback([this](){
            mMatchAnalysisWindow->SetActive(false);
        });
    }

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

    // We also need to populate the match analysis fingerprint textures...
    mSuspectInfos[0].matchAnalysisFingerprintTexture = gAssetManager.LoadTexture("S_MAD_FPRINT.BMP");
    mSuspectInfos[1].matchAnalysisFingerprintTexture = gAssetManager.LoadTexture("S_VIT_FPRINT.BMP");
    // No print for Emilio.
    mSuspectInfos[3].matchAnalysisFingerprintTexture = gAssetManager.LoadTexture("S_ABE_FPRINT.BMP");
    mSuspectInfos[4].matchAnalysisFingerprintTexture = gAssetManager.LoadTexture("S_LHO_FPRINT.BMP");
    mSuspectInfos[5].matchAnalysisFingerprintTexture = gAssetManager.LoadTexture("S_EST_FPRINT.BMP");
    mSuspectInfos[6].matchAnalysisFingerprintTexture = gAssetManager.LoadTexture("S_WIL_FPRINT.BMP");
    mSuspectInfos[7].matchAnalysisFingerprintTexture = gAssetManager.LoadTexture("S_LAR_FPRINT.BMP");
    mSuspectInfos[8].matchAnalysisFingerprintTexture = gAssetManager.LoadTexture("S_MON_FPRINT.BMP");
    mSuspectInfos[9].matchAnalysisFingerprintTexture = gAssetManager.LoadTexture("S_MOS_FPRINT.BMP");

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

    // Starting in Day 2, 2PM we show Montreaux.
    if(gGameProgress.GetTimeblock() >= Timeblock(2, 14) && mMenuBar.GetDropdownChoiceCount(1) < 9)
    {
        mMenuBar.AddDropdownChoice(1, SidneyUtil::GetSuspectsLocalizer().GetText("Menu2Item9"), [this]() {
            ShowSuspect(8);
        });
    }

    // Starting in Day 2 5PM, we MAY show Mosely, but only if his fingerprint was gathered earlier.
    if(gGameProgress.GetTimeblock() >= Timeblock(2, 17) && gGameProgress.GetFlag("GotPMoselyPrint") && mMenuBar.GetDropdownChoiceCount(1) < 10)
    {
        mMenuBar.AddDropdownChoice(1, SidneyUtil::GetSuspectsLocalizer().GetText("Menu2Item10"), [this]() {
            ShowSuspect(9);
        });

        // Also automatically link the fingerprint.
        if(!IsSuspectFingerprintLinked(9))
        {
            mSuspectInfos[9].linkedFileIds.push_back(SidneyFileIds::kMoselyFingerprint);
            mSuspectInfos[9].selectedLinkedFileIndex = 0;

            // Also show Mosely and his fingerprint as the opened data.
            mOpenedSuspectIndex = 9;
            mOpenedFileId = SidneyFileIds::kMoselyFingerprint;
        }
    }

    // Show the root.
    mRoot->SetActive(true);

    // Make sure the expected suspect and file are opened.
    ShowSuspect(mOpenedSuspectIndex);
    ShowFile(mFiles->GetFile(mOpenedFileId));
}

void SidneySuspects::Hide()
{
    // Make sure any showing file list is closed.
    mFiles->HideAllFileWindows();

    // Hide the root.
    mRoot->SetActive(false);
}

void SidneySuspects::OpenFile(int fileId)
{
    mOpenedFileId = fileId;
    ShowFile(mFiles->GetFile(mOpenedFileId));
}

void SidneySuspects::OnUpdate(float deltaTime)
{
    if(!mRoot->IsActive()) { return; }
    mMenuBar.Update();

    // This is not the most efficient approach, but I think it'll do for starters...
    // Just update the suspect notes if the input field changes.
    if(mOpenedSuspectIndex >= 0)
    {
        if(mNotesInput->GetText().size() != mSuspectInfos[mOpenedSuspectIndex].notes.size())
        {
            mSuspectInfos[mOpenedSuspectIndex].notes = mNotesInput->GetText();
        }
    }
}

void SidneySuspects::OnPersist(PersistState& ps)
{
    ps.Xfer(PERSIST_VAR(mSuspectInfos), true);
    ps.Xfer(PERSIST_VAR(mOpenedSuspectIndex));
    ps.Xfer(PERSIST_VAR(mOpenedFileId));
}

void SidneySuspects::ShowSuspect(int index)
{
    // Invalid index? Just hide the suspect data window.
    if(index < 0 || index >= mSuspectInfos.size())
    {
        mSuspectDataWindow->SetActive(false);
        return;
    }

    // Get the suspect info.
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
        mFingerprintImage->SetTexture(info.matchAnalysisFingerprintTexture);
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

    // The final option is only available if specific files are opened.
    mMenuBar.SetDropdownChoiceEnabled(2, 2, mOpenedFileId == SidneyFileIds::kUnknownLSRFingerprint ||
                                            mOpenedFileId == SidneyFileIds::kManuscriptPrint1 ||
                                            mOpenedFileId == SidneyFileIds::kManuscriptPrint2 ||
                                            mOpenedFileId == SidneyFileIds::kManuscriptPrint3);
}

bool SidneySuspects::IsSuspectFingerprintLinked(int suspectIndex)
{
    auto fingerprintFileIt = std::find(mSuspectInfos[suspectIndex].linkedFileIds.begin(),
                                       mSuspectInfos[suspectIndex].linkedFileIds.end(),
                                       mFingerprintLinkedFiles[suspectIndex].fileId);
    return fingerprintFileIt != mSuspectInfos[suspectIndex].linkedFileIds.end();
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

    // This action *may* be called from the "match analysis" popup, if it's open.
    // In that case, it should close that popup.
    mMatchAnalysisWindow->SetActive(false);
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
        info.selectedLinkedFileIndex = Math::Clamp<int>(info.selectedLinkedFileIndex, 0, info.linkedFileIds.size() - 1);
    }

    // Re-show this suspect to refresh the UI.
    ShowSuspect(mOpenedSuspectIndex);
}

void SidneySuspects::OnMatchAnalysisPressed()
{
    // Gabe doesn't do match analysis - he'll say "Grace does this stuff."
    if(StringUtil::EqualsIgnoreCase(Scene::GetEgoName(), "Gabriel"))
    {
        gActionManager.ExecuteSheepAction("wait StartDialogue(\"02O7A2ZQR1\", 1)");
        return;
    }

    // Based on the file being analyzed, decide on textures/videos and who will be a match.
    Texture* fingerprintTexture = nullptr;
    Texture* compareTexture = nullptr;
    std::string videoName;
    int matchSuspectIndex = -1;
    if(mOpenedFileId == SidneyFileIds::kUnknownLSRFingerprint)
    {
        fingerprintTexture = gAssetManager.LoadTexture("EST_LSR_PRINT.BMP", AssetScope::Scene);
        compareTexture = gAssetManager.LoadTexture("EST_LSR_COMPARE.BMP", AssetScope::Scene);
        videoName = "EstLSRScan.avi";
        matchSuspectIndex = 5;
    }
    else if(mOpenedFileId == SidneyFileIds::kManuscriptPrint1)
    {
        fingerprintTexture = gAssetManager.LoadTexture("BUCH_BLD_PRINT.BMP", AssetScope::Scene);
        compareTexture = gAssetManager.LoadTexture("BUCH_BLD_COMPARE.BMP", AssetScope::Scene);
        videoName = "BuchBldScan.avi";
        matchSuspectIndex = 1;
    }
    else if(mOpenedFileId == SidneyFileIds::kManuscriptPrint2)
    {
        fingerprintTexture = gAssetManager.LoadTexture("BUTH_BLD_PRINT.BMP", AssetScope::Scene);
        compareTexture = gAssetManager.LoadTexture("BUTH_BLD_COMPARE.BMP", AssetScope::Scene);
        videoName = "ButhBldScan.avi";
        matchSuspectIndex = 0;
    }
    else if(mOpenedFileId == SidneyFileIds::kManuscriptPrint3)
    {
        fingerprintTexture = gAssetManager.LoadTexture("MOS_BLD_PRINT.BMP", AssetScope::Scene);
        compareTexture = gAssetManager.LoadTexture("MOS_BLD_COMPARE.BMP", AssetScope::Scene);
        videoName = "MosBldScan.avi";
        matchSuspectIndex = 9;
    }

    // Show popup in its initial state.
    mMAFingerprintImage->SetTexture(fingerprintTexture);
    mMAFingerprintVideoImage->SetEnabled(false);
    mMAActionLabel->SetText(SidneyUtil::GetSuspectsLocalizer().GetText("MatchAnalyze"));
    mMASuspectLabel->SetText("");
    mMALinkToSuspectButton->GetButton()->SetCanInteract(false);
    mMACloseButton->GetButton()->SetCanInteract(false);
    mMatchAnalysisWindow->SetActive(true);

    // The game is in hands-off mode until the match analysis completes.
    gActionManager.StartManualAction();

    // Ok, we're going to put on a big show of analyzing the fingerprint and comparing it to all the suspects!
    gAudioManager.PlaySFX(gAssetManager.LoadAudio("WORKING3.WAV", AssetScope::Scene), [this, videoName, compareTexture, matchSuspectIndex](){

        // Play video file of the match analysis occurring.
        mMAFingerprintVideoImage->SetEnabled(true);
        mMAFingerprintVideoImage->Play(videoName, [this, compareTexture, matchSuspectIndex](){

            // Once video completes, the video image shows the compare texture overlay.
            mMAFingerprintVideoImage->SetTexture(compareTexture);

            // Now we move on to compare mode...
            mMAActionLabel->SetText(SidneyUtil::GetSuspectsLocalizer().GetText("MatchCompare"));
            OnMatchAnalysisCheckSuspect(0, matchSuspectIndex);
        });
    });
}

void SidneySuspects::OnMatchAnalysisCheckSuspect(int currentIndex, int matchIndex)
{
    // If we've iterated all suspects and not found a match, say so.
    if(currentIndex >= mSuspectInfos.size())
    {
        mMAActionLabel->SetText(SidneyUtil::GetSuspectsLocalizer().GetText("MatchNone"));
        mMASuspectLabel->SetText("");
        mMACloseButton->GetButton()->SetCanInteract(true);

        // Completes the match analysis action.
        gActionManager.FinishManualAction();

        // If this is the unknown LSR fingerprint, Grace plays some dialogue about having to try again later.
        if(mOpenedFileId == SidneyFileIds::kUnknownLSRFingerprint)
        {
            if(!gGameProgress.GetFlag("PlayedEstelleFPDialog"))
            {
                gActionManager.ExecuteDialogueAction("02OX660SL1");
                gGameProgress.SetFlag("PlayedEstelleFPDialog");
            }
        }
        return;
    }

    // If no fingerprint is linked to this suspect, we can skip them right away.
    if(!IsSuspectFingerprintLinked(currentIndex))
    {
        OnMatchAnalysisCheckSuspect(currentIndex + 1, matchIndex);
        return;
    }

    // Show this suspect in the UI to show that we are considering them.
    mMASuspectLabel->SetText(mSuspectInfos[currentIndex].name);
    mMAFingerprintImage->SetTexture(mSuspectInfos[currentIndex].matchAnalysisFingerprintTexture);

    // Pause for a moment before showing result or moving on to next person.
    Timers::AddTimerSeconds(2.0f, [this, currentIndex, matchIndex](){
        // If we found a match, say so!
        if(currentIndex == matchIndex)
        {
            // Say that we found a match.
            mMAActionLabel->SetText(SidneyUtil::GetSuspectsLocalizer().GetText("MatchFound"));

            // Allow pressing either link or close buttons.
            mMALinkToSuspectButton->GetButton()->SetCanInteract(true);
            mMACloseButton->GetButton()->SetCanInteract(true);

            // Disable the video image.
            mMAFingerprintVideoImage->SetEnabled(false);

            // Open the suspect's page.
            ShowSuspect(currentIndex);

            // Completes the match analysis action.
            gActionManager.FinishManualAction();

            // Do some action that depends on the file matched.
            if(mOpenedFileId == SidneyFileIds::kManuscriptPrint1)
            {
                // Grace says "Buchelli's a sneak!"
                gActionManager.ExecuteDialogueAction("027X65Q3L2");
                gGameProgress.ChangeScore("e_sidney_analysis_link_manuscript_prints_buchelli");
                gGameProgress.SetFlag("MatchedBuchelli");
                mMAFingerprintImage->SetTexture(gAssetManager.LoadTexture("BUCH_RED_BLD_PRINT.BMP", AssetScope::Scene));
            }
            else if(mOpenedFileId == SidneyFileIds::kManuscriptPrint2)
            {
                // Grace says "Madeline had her hands on the manuscript!"
                gActionManager.ExecuteDialogueAction("027X65Q3L1");
                gGameProgress.ChangeScore("e_sidney_analysis_link_manuscript_prints_buthane");
                gGameProgress.SetFlag("MatchedButhane");
                mMAFingerprintImage->SetTexture(gAssetManager.LoadTexture("BUTH_RED_BLD_PRINT.BMP", AssetScope::Scene));
            }
            else if(mOpenedFileId == SidneyFileIds::kManuscriptPrint3)
            {
                // Grace says "Mosely! Gabriel was right!"
                gActionManager.ExecuteDialogueAction("027X65Q3L3");
                gGameProgress.ChangeScore("e_sidney_analysis_link_manuscript_prints_mosley");
                gGameProgress.SetFlag("MatchedMosely");
                mMAFingerprintImage->SetTexture(gAssetManager.LoadTexture("MOS_RED_BLD_PRINT.BMP", AssetScope::Scene));
            }
            else if(mOpenedFileId == SidneyFileIds::kUnknownLSRFingerprint)
            {
                // Grace says "Estelle!? WTH???"
                gActionManager.ExecuteDialogueAction("02OX660SJ1");
                gGameProgress.ChangeScore("e_sidney_suspect_link_fingerprint_lsr_unknown");
                gGameProgress.SetFlag("MatchedEstelle");
                mMAFingerprintImage->SetTexture(gAssetManager.LoadTexture("EST_RED_LSR_PRINT.BMP", AssetScope::Scene));
            }
        }
        else
        {
            // Not a match - move on to next suspect.
            OnMatchAnalysisCheckSuspect(currentIndex + 1, matchIndex);
        }
    });
}

void SidneySuspects::OnLinkedEvidenceSelected(int index)
{
    // Change the selected file index.
    mSuspectInfos[mOpenedSuspectIndex].selectedLinkedFileIndex = index;

    // Re-show the suspect info to change the highlight.
    ShowSuspect(mOpenedSuspectIndex);
}
