#include "SidneyAnalyze.h"

#include "AssetManager.h"
#include "Sidney.h"
#include "SidneyButton.h"
#include "SidneyFiles.h"
#include "SidneyPopup.h"
#include "SidneyUtil.h"
#include "UIButton.h"
#include "UICanvas.h"
#include "UINineSlice.h"
#include "UIImage.h"

void SidneyAnalyze::Init(Sidney* sidney, SidneyFiles* sidneyFiles)
{
    mSidney = sidney;
    mSidneyFiles = sidneyFiles;

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
    mMenuBar.Init(mRoot, SidneyUtil::GetAnalyzeLocalizer().GetText("ScreenName"), 120.0f);
    mMenuBar.SetFirstDropdownPosition(24.0f);
    mMenuBar.SetDropdownSpacing(26.0f);

    // "Open" dropdown.
    mMenuBar.AddDropdown(SidneyUtil::GetAnalyzeLocalizer().GetText("Menu1Name"));
    {
        // "Open File" choice.
        mMenuBar.AddDropdownChoice(SidneyUtil::GetAnalyzeLocalizer().GetText("MenuItem1"), [this](){

            // Show the file selector.
            mSidneyFiles->Show([this](SidneyFile* selectedFile){
                mAnalyzeFileId = selectedFile->id;

                // If the file has never been analyzed before, we show the pre-analyze UI.
                // Otherwise, we can go to the appropriate state directly.
                if(!selectedFile->hasBeenAnalyzed)
                {
                    SetState(SidneyAnalyze::State::PreAnalyze);
                }
                else
                {
                    SetStateFromFile();
                }
            });
        });
    }

    // "Text" dropdown.
    mMenuBar.AddDropdown(SidneyUtil::GetAnalyzeLocalizer().GetText("Menu2Name"));
    {
        // "Extract Anomalies" choice.
        mMenuBar.AddDropdownChoice(SidneyUtil::GetAnalyzeLocalizer().GetText("Menu2Item1"), nullptr);

        // "Translate" choice.
        mMenuBar.AddDropdownChoice(SidneyUtil::GetAnalyzeLocalizer().GetText("Menu2Item2"), nullptr);

        // "Anagram Parser" choice.
        mMenuBar.AddDropdownChoice(SidneyUtil::GetAnalyzeLocalizer().GetText("Menu2Item3"), nullptr);

        // "Analyze Text" choice.
        mMenuBar.AddDropdownChoice(SidneyUtil::GetAnalyzeLocalizer().GetText("Menu2Item4"), nullptr);
    }

    // "Graphic" dropdown.
    mMenuBar.AddDropdown(SidneyUtil::GetAnalyzeLocalizer().GetText("Menu3Name"));
    {
        // "View Geometry" choice.
        mMenuBar.AddDropdownChoice(SidneyUtil::GetAnalyzeLocalizer().GetText("Menu3Item1"), [this](){
            AnalyzeImage_OnViewGeometryButtonPressed();
        });

        // "Rotate Shape" choice.
        mMenuBar.AddDropdownChoice(SidneyUtil::GetAnalyzeLocalizer().GetText("Menu3Item2"), nullptr);

        // "Zoom & Clarify" choice.
        mMenuBar.AddDropdownChoice(SidneyUtil::GetAnalyzeLocalizer().GetText("Menu3Item3"), [this](){
            AnalyzeImage_OnZoomClarifyButtonPressed();
        });

        //TODO: Add a divider/empty space here. (Menu3Item4)

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

        //TODO: Add a divider/empty space here. (Menu4Item3)

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
        mPreAnalyzeWindow = new Actor(TransformType::RectTransform);
        mPreAnalyzeWindow->GetTransform()->SetParent(mRoot->GetTransform());

        UINineSlice* border = mPreAnalyzeWindow->AddComponent<UINineSlice>(SidneyUtil::GetGrayBoxParams(SidneyUtil::TransBgColor));
        border->GetRectTransform()->SetSizeDelta(153.0f, 167.0f);

        border->GetRectTransform()->SetAnchor(AnchorPreset::BottomLeft);
        border->GetRectTransform()->SetAnchoredPosition(40.0f, 203.0f);

        // Add one line for the box header.
        Actor* boxHeaderDividerActor = new Actor(TransformType::RectTransform);
        boxHeaderDividerActor->GetTransform()->SetParent(mPreAnalyzeWindow->GetTransform());

        UIImage* boxHeaderDividerImage = boxHeaderDividerActor->AddComponent<UIImage>();
        boxHeaderDividerImage->SetTexture(gAssetManager.LoadTexture("S_BOX_TOP.BMP"), true);
        boxHeaderDividerImage->GetRectTransform()->SetPivot(0.5f, 1.0f);
        boxHeaderDividerImage->GetRectTransform()->SetAnchorMin(0.0f, 1.0f);
        boxHeaderDividerImage->GetRectTransform()->SetAnchorMax(1.0f, 1.0f);
        boxHeaderDividerImage->GetRectTransform()->SetAnchoredPosition(0.0f, -17.0f);
        boxHeaderDividerImage->GetRectTransform()->SetSizeDeltaX(0.0f);

        // Add header label.
        Actor* headerActor = new Actor(TransformType::RectTransform);
        headerActor->GetTransform()->SetParent(mPreAnalyzeWindow->GetTransform());

        mPreAnalyzeTitleLabel = headerActor->AddComponent<UILabel>();
        mPreAnalyzeTitleLabel->SetFont(gAssetManager.LoadFont("SID_TEXT_14.FON"));
        mPreAnalyzeTitleLabel->SetText("Map");
        mPreAnalyzeTitleLabel->SetHorizonalAlignment(HorizontalAlignment::Right);
        mPreAnalyzeTitleLabel->SetVerticalAlignment(VerticalAlignment::Top);
        mPreAnalyzeTitleLabel->SetMasked(true);

        mPreAnalyzeTitleLabel->GetRectTransform()->SetAnchor(AnchorPreset::TopRight);
        mPreAnalyzeTitleLabel->GetRectTransform()->SetAnchoredPosition(-8.0f, -3.0f);
        mPreAnalyzeTitleLabel->GetRectTransform()->SetSizeDeltaY(14.0f);

        // Add item image.
        Actor* imageActor = new Actor(TransformType::RectTransform);
        imageActor->GetTransform()->SetParent(mPreAnalyzeWindow->GetTransform());

        mPreAnalyzeItemImage = imageActor->AddComponent<UIImage>();
        mPreAnalyzeItemImage->SetTexture(gAssetManager.LoadTexture("MAP9.BMP"), true);
        mPreAnalyzeItemImage->GetRectTransform()->SetAnchoredPosition(0.0f, -8.0f);

        // Hide pre-analyze window by default.
        mPreAnalyzeWindow->SetActive(false);
    }

    // Analyze message box.
    mAnalyzePopup = new SidneyPopup(mRoot);
    
    // Start in empty state.
    SetState(State::Empty);

    // Hide by default.
    Hide();
}

void SidneyAnalyze::Show()
{
    mRoot->SetActive(true);

    // Make sure the state of the screen is up to date when entering.
    // This is especially important to restore the UI state after loading a save game.
    SetState(mState);
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

    // Save the state.
    mState = state;

    // Update UI based on current state.
    switch(mState)
    {
    case State::Empty:
        // Everything is already turned off, so nothing to do.
        break;

    case State::PreAnalyze:
    {
        // Show the pre-analyze UI with appropriate text/image for currently selected file.
        mPreAnalyzeWindow->SetActive(true);

        SidneyFile* file = mSidneyFiles->GetFile(mAnalyzeFileId);
        if(file != nullptr)
        {
            mPreAnalyzeTitleLabel->SetText(file->GetDisplayName());
            mPreAnalyzeItemImage->SetTexture(file->GetIcon());
        }
        break;
    }

    case State::Map:
        AnalyzeMap_EnterState();
        break;

    case State::Image:
        AnalyzeImage_EnterState();
        break;

    case State::Text:
        AnalyzeText_EnterState();
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

void SidneyAnalyze::ShowAnalyzeMessage(const std::string& message, const Vector2& position, HorizontalAlignment textAlignment)
{
    mAnalyzePopup->ResetToDefaults();
    mAnalyzePopup->SetWindowPosition(position);

    mAnalyzePopup->SetTextAlignment(textAlignment);
    mAnalyzePopup->SetText(message);
    
    mAnalyzePopup->ShowOneButton();
}