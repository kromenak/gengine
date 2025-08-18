#include "SidneyAnalyze.h"

#include "Sidney.h"
#include "SidneyButton.h"
#include "SidneyFakeInputPopup.h"
#include "SidneyFiles.h"
#include "SidneyPopup.h"
#include "SidneyUtil.h"
#include "UIButton.h"
#include "UIImage.h"

void SidneyAnalyze::Init(Sidney* sidney, SidneyFiles* sidneyFiles, SidneyTranslate* sidneyTranslate)
{
    mSidney = sidney;
    mSidneyFiles = sidneyFiles;
    mSidneyTranslate = sidneyTranslate;

    // Add background. This will also be the root for this screen.
    mRoot = SidneyUtil::CreateBackground(mSidney);
    mRoot->SetName("Analyze");

    // Add main menu button.
    SidneyUtil::CreateMainMenuButton(mRoot, [this](){
        Hide();
    });

    // Because the menu bar hangs down *in front of* analysis views, AND because the UI system doesn't yet have good ordering tools...
    // ...we should create the anaysis views *first* so they appear *behind* the menu bar.
    AnalyzeMap_Init();
    AnalyzeImage_Init();
    AnalyzeText_Init();

    // Add menu bar.
    mMenuBar.Init(mRoot, SidneyUtil::GetAnalyzeLocalizer().GetText("ScreenName"));
    mMenuBar.SetFirstDropdownPosition(24.0f);
    mMenuBar.SetDropdownSpacing(26.0f);

    // "Open" dropdown.
    mMenuBar.AddDropdown(SidneyUtil::GetAnalyzeLocalizer().GetText("Menu1Name"));
    {
        // "Open File" choice.
        mMenuBar.AddDropdownChoice(SidneyUtil::GetAnalyzeLocalizer().GetText("MenuItem1"), [this](){

            // Show the file selector.
            mSidneyFiles->Show([this](SidneyFile* selectedFile){
                OpenFile(selectedFile->id);
            });
        });
    }

    // "Text" dropdown.
    mMenuBar.AddDropdown(SidneyUtil::GetAnalyzeLocalizer().GetText("Menu2Name"));
    {
        // "Extract Anomalies" choice.
        mMenuBar.AddDropdownChoice(SidneyUtil::GetAnalyzeLocalizer().GetText("Menu2Item1"), [this](){
            AnalyzeImage_OnExtractAnomoliesPressed();
        });

        // "Translate" choice.
        mMenuBar.AddDropdownChoice(SidneyUtil::GetAnalyzeLocalizer().GetText("Menu2Item2"), [this](){
            AnalyzeText_OnTranslateButtonPressed();
        });

        // "Anagram Parser" choice.
        mMenuBar.AddDropdownChoice(SidneyUtil::GetAnalyzeLocalizer().GetText("Menu2Item3"), [this](){
            AnalyzeText_OnAnagramParserPressed();
        });

        // "Analyze Text" choice.
        mMenuBar.AddDropdownChoice(SidneyUtil::GetAnalyzeLocalizer().GetText("Menu2Item4"), [this](){
            AnalyzeImage_OnAnalyzeTextPressed();
        });
    }

    // "Graphic" dropdown.
    mMenuBar.AddDropdown(SidneyUtil::GetAnalyzeLocalizer().GetText("Menu3Name"));
    {
        // "View Geometry" choice.
        mMenuBar.AddDropdownChoice(SidneyUtil::GetAnalyzeLocalizer().GetText("Menu3Item1"), [this](){
            AnalyzeImage_OnViewGeometryButtonPressed();
        });

        // "Rotate Shape" choice.
        mMenuBar.AddDropdownChoice(SidneyUtil::GetAnalyzeLocalizer().GetText("Menu3Item2"), [this](){
            AnalyzeImage_OnRotateShapeButtonPressed();
        });

        // "Zoom & Clarify" choice.
        mMenuBar.AddDropdownChoice(SidneyUtil::GetAnalyzeLocalizer().GetText("Menu3Item3"), [this](){
            AnalyzeImage_OnZoomClarifyButtonPressed();
        });

        mMenuBar.AddDropdownChoiceSeparator();

        // "Use Shape" choice.
        mMenuBar.AddDropdownChoice(SidneyUtil::GetAnalyzeLocalizer().GetText("Menu3Item5"), [this](){
            AnalyzeMap_OnUseShapePressed();
        });

        // (Note: "Save Shape" is Item6, but I don't think it is used in the final game?)

        // "Erase Shape" choice.
        mMenuBar.AddDropdownChoice(SidneyUtil::GetAnalyzeLocalizer().GetText("Menu3Item7"), [this](){
            AnalyzeMap_OnEraseShapePressed();
        });
    }

    // "Map" dropdown.
    mMenuBar.AddDropdown(SidneyUtil::GetAnalyzeLocalizer().GetText("Menu4Name"));
    {
        // "Enter Points" choice.
        mMenuBar.AddDropdownChoice(SidneyUtil::GetAnalyzeLocalizer().GetText("Menu4Item1"), [this](){
            AnalyzeMap_OnEnterPointsPressed();
        });

        // "Clear Points" choice.
        mMenuBar.AddDropdownChoice(SidneyUtil::GetAnalyzeLocalizer().GetText("Menu4Item2"), [this](){
            AnalyzeMap_OnClearPointsPressed();
        });

        mMenuBar.AddDropdownChoiceSeparator();

        // "Draw Grid" choice.
        mMenuBar.AddDropdownChoice(SidneyUtil::GetAnalyzeLocalizer().GetText("Menu4Item4"), [this](){
            AnalyzeMap_OnDrawGridPressed();
        });

        // "Erase Grid" choice.
        mMenuBar.AddDropdownChoice(SidneyUtil::GetAnalyzeLocalizer().GetText("Menu4Item5"), [this](){
            AnalyzeMap_OnEraseGridPressed();
        });
    }

    // Add analyze button.
    {
        SidneyButton* analyzeButton = SidneyUtil::CreateBigButton(mRoot);
        analyzeButton->SetName("AnalyzeButton");
        analyzeButton->SetText(SidneyUtil::GetAnalyzeLocalizer().GetText("MenuItem2"));
        float labelWidth = analyzeButton->GetLabel()->GetTextWidth() + 12.0f;
        analyzeButton->SetWidth(labelWidth);

        analyzeButton->GetRectTransform()->SetPivot(1.0f, 0.0f); // Bottom-Right
        analyzeButton->GetRectTransform()->SetAnchor(1.0f, 0.0f); // Bottom-Right
        analyzeButton->GetRectTransform()->SetAnchoredPosition(-113.0f, 10.0f); // offset from bottom-right

        analyzeButton->GetButton()->SetCanInteract(false);

        mAnalyzeButton = analyzeButton;
        mAnalyzeButton->SetPressCallback([this](){
            OnAnalyzeButtonPressed();
        });
    }

    // Add pre-analyze window.
    {
        FilePreviewWindow win = SidneyUtil::CreateFilePreviewWindow(mRoot);
        mPreAnalyzeWindow = win.root;
        mPreAnalyzeTitleLabel = win.header;
        mPreAnalyzeItemImage = win.image;

        mPreAnalyzeWindow->GetComponent<RectTransform>()->SetAnchor(AnchorPreset::BottomLeft);
        mPreAnalyzeWindow->GetComponent<RectTransform>()->SetAnchoredPosition(40.0f, 203.0f);

        // Hide pre-analyze window by default.
        mPreAnalyzeWindow->SetActive(false);
    }

    // Analyze message box.
    mAnalyzePopup = new SidneyPopup(mRoot);
    mSecondaryAnalyzePopup = new SidneyPopup(mRoot);
    mSetTextPopup = new SidneyFakeInputPopup(mRoot, "Set Text Popup");

    // Start in empty state.
    SetState(State::Empty);

    // Hide by default.
    Hide();
}

void SidneyAnalyze::Show(int openFileId)
{
    mRoot->SetActive(true);

    // Make sure the state of the screen is up to date when entering.
    // This is especially important to restore the UI state after loading a save game.
    if(openFileId == -1 || openFileId == mAnalyzeFileId)
    {
        SetState(mState);
    }
    else
    {
        OpenFile(openFileId);
    }
}

void SidneyAnalyze::Hide()
{
    mRoot->SetActive(false);
}

void SidneyAnalyze::OnUpdate(float deltaTime)
{
    // Only update this screen if it's active.
    if(!mRoot->IsActive()) { return; }

    // Update menu bar.
    mMenuBar.Update();

    // Update analyze sub-views.
    AnalyzeMap_Update(deltaTime);
}

void SidneyAnalyze::OnPersist(PersistState& ps)
{
    ps.Xfer<State, int>(PERSIST_VAR(mState));
    ps.Xfer(PERSIST_VAR(mMap));
    ps.Xfer(PERSIST_VAR(mAnalyzeFileId));

    // HACK: Ensure LSR state var is up-to-date, for save games that were made before we started tracking this.
    SidneyUtil::UpdateLSRState();

    // Older save games may have an incorrect map layout - run some code to detect and fix this.
    if(ps.GetFormatVersionNumber() < 3)
    {
        AnalyzeMap_FixOldSaveGames();
    }
}

void SidneyAnalyze::OpenFile(int fileId)
{
    // Save ID of file that's now being analyzed.
    mAnalyzeFileId = fileId;

    // If the file has never been analyzed before, we show the pre-analyze UI.
    // Otherwise, we can go to the appropriate state directly.
    SidneyFile* file = mSidneyFiles->GetFile(mAnalyzeFileId);
    if(file == nullptr)
    {
        SetState(State::Empty);
    }
    else if(!file->hasBeenAnalyzed)
    {
        SetState(State::PreAnalyze);
    }
    else
    {
        SetStateFromFile();
    }
}

void SidneyAnalyze::SetState(State state)
{
    // Turn everything off for starters.
    mPreAnalyzeWindow->SetActive(false);
    mAnalyzeMapWindow->SetActive(false);
    mAnalyzeImageWindow->SetActive(false);
    mAnalyzeTextWindow->SetActive(false);

    // All dropdowns except "Open" are disabled by default.
    mMenuBar.SetDropdownEnabled(kFileDropdownIdx, true);
    mMenuBar.SetDropdownEnabled(kTextDropdownIdx, false);
    mMenuBar.SetDropdownEnabled(kGraphicDropdownIdx, false);
    mMenuBar.SetDropdownEnabled(kMapDropdownIdx, false);

    // If needed, notify state that we're leaving.
    switch(mState)
    {
    case State::Text:
        AnalyzeText_ExitState();
        break;

    default:
        break;
    }

    // Save the state.
    mState = state;

    // Update UI based on current state.
    switch(mState)
    {
    case State::Empty:
        // Everything is already turned off, so nothing to do.
        break;

    case State::PreAnalyze:
        ShowPreAnalyzeUI();
        break;

    case State::Map:
        AnalyzeMap_EnterState();
        break;

    case State::Image:
        AnalyzeImage_EnterState();
        break;

    case State::Text:
        AnalyzeText_EnterState();
        break;

    case State::Audio:
        ShowPreAnalyzeUI();
        mMenuBar.SetDropdownEnabled(kTextDropdownIdx, true);
        mMenuBar.SetDropdownChoiceEnabled(kTextDropdownIdx, kTextDropdown_ExtractAnomaliesIdx, false);
        mMenuBar.SetDropdownChoiceEnabled(kTextDropdownIdx, kTextDropdown_TranslateIdx, true);
        mMenuBar.SetDropdownChoiceEnabled(kTextDropdownIdx, kTextDropdown_AnagramParserIdx, false);
        mMenuBar.SetDropdownChoiceEnabled(kTextDropdownIdx, kTextDropdown_AnalyzeTextIdx, false);
        break;
    }

    // The analyze button is clickable as long as we aren't in the "Empty" state.
    mAnalyzeButton->GetButton()->SetCanInteract(mState != State::Empty);
}

void SidneyAnalyze::SetStateFromFile()
{
    // No file selected - show empty.
    SidneyFile* file = mSidneyFiles->GetFile(mAnalyzeFileId);
    if(file == nullptr)
    {
        SetState(State::Empty);
        return;
    }
    else if(file->id == SidneyFileIds::kMap)
    {
        SetState(State::Map);
    }
    else if(file->type == SidneyFileType::Image)
    {
        SetState(State::Image);
    }
    else if(file->type == SidneyFileType::Text)
    {
        SetState(State::Text);
    }
    else if(file->type == SidneyFileType::Audio)
    {
        SetState(State::Audio);
    }
    else
    {
        printf("Unknown analyze state!\n");
    }
}

void SidneyAnalyze::OnAnalyzeButtonPressed()
{
    // We need a file to analyze.
    if(mAnalyzeFileId == -1) { return; }

    // Set to right state depending on the file we're analyzing.
    SetStateFromFile();

    // Take the appropriate analyze action based on the item.
    switch(mState)
    {
    default:
    case State::Empty:
    case State::PreAnalyze:
        // Do nothing in these cases - Analyze button has no effect.
        break;

    case State::Map:
        AnalyzeMap_OnAnalyzeButtonPressed();
        break;

    case State::Image:
        AnalyzeImage_OnAnalyzeButtonPressed();
        break;

    case State::Text:
        AnalyzeText_OnAnalyzeButtonPressed();
        break;

    case State::Audio:
        ShowAnalyzeMessage("AnalyzeTape", Vector2(), HorizontalAlignment::Center);
        break;
    }

    // There's a chance attempting to analyze an image will turn up nothing interesting.
    // This will "kick us back out" to the PreAnalyze state.
    // In this case, we should early out and consider the file "not analyzed" (so we don't go directly to analyze state for this file later on).
    if(mState == State::PreAnalyze)
    {
        return;
    }

    // This file has definitely been analyzed at least once now!
    SidneyFile* file = mSidneyFiles->GetFile(mAnalyzeFileId);
    if(file != nullptr)
    {
        file->hasBeenAnalyzed = true;
    }
}

void SidneyAnalyze::ShowPreAnalyzeUI()
{
    // Show the pre-analyze UI with appropriate text/image for currently selected file.
    mPreAnalyzeWindow->SetActive(true);

    SidneyFile* file = mSidneyFiles->GetFile(mAnalyzeFileId);
    if(file != nullptr)
    {
        mPreAnalyzeTitleLabel->SetText(file->GetDisplayName());
        mPreAnalyzeItemImage->SetTexture(file->GetIcon());
    }
}

void SidneyAnalyze::ShowAnalyzeMessage(const std::string& message, const Vector2& position, HorizontalAlignment textAlignment, bool noButtons)
{
    mAnalyzePopup->ResetToDefaults();
    mAnalyzePopup->SetWindowPosition(position);

    mAnalyzePopup->SetTextAlignment(textAlignment);
    mAnalyzePopup->SetText(SidneyUtil::GetAnalyzeLocalizer().GetText(message));

    if(noButtons)
    {
        mAnalyzePopup->ShowNoButton();
    }
    else
    {
        mAnalyzePopup->ShowOneButton();
    }
}