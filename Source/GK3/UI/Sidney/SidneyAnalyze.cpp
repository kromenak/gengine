#include "SidneyAnalyze.h"

#include "ActionManager.h"
#include "AssetManager.h"
#include "CursorManager.h"
#include "GameProgress.h"
#include "InventoryManager.h"
#include "SidneyButton.h"
#include "SidneyFiles.h"
#include "SidneyUtil.h"
#include "UIButton.h"
#include "UICanvas.h"
#include "UICircles.h"
#include "UINineSlice.h"
#include "UIImage.h"
#include "UILines.h"
#include "UIPoints.h"

void SidneyAnalyze::Init(Actor* parent, SidneyFiles* sidneyFiles)
{
    mSidneyFiles = sidneyFiles;

    // Add background. This will also be the root for this screen.
    mRoot = SidneyUtil::CreateBackground(parent);
    mRoot->SetName("Email");

    // Add main menu button.
    SidneyUtil::CreateMainMenuButton(mRoot, [this](){
        Hide();
    });

    // Because the menu bar hangs down *in front of* analysis views, AND because the UI system doesn't yet have good ordering tools...
    // ...we should create the anaysis views *first* so they appear *behind* the menu bar.
    AnalyzeMap_Init();
    AnalyzeImageInit();

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
                mAnalyzeFile = selectedFile;

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
            printf("View Geometry\n");

            if(mAnalyzeFile != nullptr)
            {
                if(mAnalyzeFile->index == 20)
                {
                    printf("Added triangle\n");
                    mSidneyFiles->AddFile(37); // Triangle
                }
                if(mAnalyzeFile->index == 21)
                {
                    printf("Added circle and square\n");
                    mSidneyFiles->AddFile(38); // Circle
                    mSidneyFiles->AddFile(39); // Square
                }
            }
        });

        // "Rotate Shape" choice.
        mMenuBar.AddDropdownChoice(SidneyUtil::GetAnalyzeLocalizer().GetText("Menu3Item2"), nullptr);

        // "Zoom & Clarify" choice.
        mMenuBar.AddDropdownChoice(SidneyUtil::GetAnalyzeLocalizer().GetText("Menu3Item3"), nullptr);

        //TODO: Add a divider/empty space here. (Menu3Item4)

        // "Use Shape" choice.
        mMenuBar.AddDropdownChoice(SidneyUtil::GetAnalyzeLocalizer().GetText("Menu3Item5"), [this](){
            mSidneyFiles->ShowShapes([this](SidneyFile* selectedFile){
                mMap.AddShape(selectedFile->name);
            });
        });

        // (Note: "Save Shape" is Item6, but I don't think it is used in the final game?)

        // "Erase Shape" choice.
        mMenuBar.AddDropdownChoice(SidneyUtil::GetAnalyzeLocalizer().GetText("Menu3Item7"), [this](){
            if(mMap.IsShapeSelected())
            {
                mMap.EraseShape();
            }
            else
            {
                ShowAnalyzeMessage("NoShapeNote");
            }
        });
    }

    // "Map" dropdown.
    mMenuBar.AddDropdown(SidneyUtil::GetAnalyzeLocalizer().GetText("Menu4Name"));
    {
        // "Enter Points" choice.
        mMenuBar.AddDropdownChoice(SidneyUtil::GetAnalyzeLocalizer().GetText("Menu4Item1"), [this](){

            // Not allowed to place points until you've progressed far enough in the story for Grace to have something to plot.
            if(gGameProgress.GetTimeblock() == Timeblock("205P"))
            {
                // The game wants you to pick up these items before it thinks you have enough info to place points.
                if(!gInventoryManager.HasInventoryItem("Church_Pamphlet") || !gInventoryManager.HasInventoryItem("LSR"))
                {
                    gActionManager.ExecuteSheepAction("wait StartDialogue(\"02O8O2ZPI1\", 1)");
                    return;
                }
            }

            // Otherwise, we can place points.
            AnalyzeMap_SetStatusText(SidneyUtil::GetAnalyzeLocalizer().GetText("EnterPointsNote"));
            mEnteringPoints = true;
        });

        // "Clear Points" choice.
        mMenuBar.AddDropdownChoice(SidneyUtil::GetAnalyzeLocalizer().GetText("Menu4Item2"), [this](){
            mEnteringPoints = false;
            mMap.zoomedIn.points->ClearPoints();
            mMap.zoomedOut.points->ClearPoints();
        });

        //TODO: Add a divider/empty space here. (Menu4Item3)

        // "Draw Grid" choice.
        mMenuBar.AddDropdownChoice(SidneyUtil::GetAnalyzeLocalizer().GetText("Menu4Item4"), nullptr);

        // "Erase Grid" choice.
        mMenuBar.AddDropdownChoice(SidneyUtil::GetAnalyzeLocalizer().GetText("Menu4Item5"), nullptr);
    }

    // All dropdowns except "Open" are disabled by default.
    mMenuBar.SetDropdownEnabled(1, false);
    mMenuBar.SetDropdownEnabled(2, false);
    mMenuBar.SetDropdownEnabled(3, false);

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
    {
        mAnalyzeMessageWindow = new Actor(TransformType::RectTransform);
        mAnalyzeMessageWindow->GetTransform()->SetParent(mRoot->GetTransform());

        UINineSlice* border = mAnalyzeMessageWindow->AddComponent<UINineSlice>(SidneyUtil::GetGrayBoxParams(Color32::Black));
        border->GetRectTransform()->SetSizeDelta(250.0f, 157.0f);
        border->GetRectTransform()->SetAnchor(AnchorPreset::Center);
        border->GetRectTransform()->SetAnchoredPosition(0.0f, 0.0f);

        // Add message label.
        Actor* messageActor = new Actor(TransformType::RectTransform);
        messageActor->GetTransform()->SetParent(mAnalyzeMessageWindow->GetTransform());

        mAnalyzeMessage = messageActor->AddComponent<UILabel>();
        mAnalyzeMessage->SetFont(gAssetManager.LoadFont("SID_TEXT_14.FON"));
        mAnalyzeMessage->SetHorizonalAlignment(HorizontalAlignment::Center);
        mAnalyzeMessage->SetHorizontalOverflow(HorizontalOverflow::Wrap);
        mAnalyzeMessage->SetVerticalAlignment(VerticalAlignment::Top);
        
        mAnalyzeMessage->GetRectTransform()->SetAnchor(AnchorPreset::CenterStretch);
        mAnalyzeMessage->GetRectTransform()->SetAnchoredPosition(0.0f, 0.0f);
        mAnalyzeMessage->GetRectTransform()->SetSizeDelta(-15.0f, -15.0f);

        SidneyButton* okButton = new SidneyButton(mAnalyzeMessageWindow);
        okButton->SetFont(gAssetManager.LoadFont("SID_TEXT_14.FON"));
        okButton->SetText(SidneyUtil::GetAnalyzeLocalizer().GetText("OKButton"));
        okButton->SetWidth(80.0f);

        okButton->GetRectTransform()->SetAnchor(AnchorPreset::Bottom);
        okButton->GetRectTransform()->SetAnchoredPosition(0.0f, 6.0f);

        okButton->SetPressCallback([this](){
            mAnalyzeMessageWindow->SetActive(false);
        });
        mAnalyzeMessageWindow->SetActive(false);
    }

    // Hide by default.
    Hide();
}

void SidneyAnalyze::Show()
{
    mRoot->SetActive(true);
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

void SidneyAnalyze::SetState(State state)
{
    // Turn everything off for starters.
    mPreAnalyzeWindow->SetActive(false);
    mAnalyzeMapWindow->SetActive(false);
    mAnalyzeImageWindow->SetActive(false);

    // Save the state.
    mState = state;

    // Update UI based on current state.
    switch(mState)
    {
    case State::Empty:
        // Everything is already turned off, so nothing to do.
        break;

    case State::PreAnalyze:
        // Show the pre-analyze UI with appropriate text/image for currently selected file.
        mPreAnalyzeWindow->SetActive(true);
        mPreAnalyzeTitleLabel->SetText(mAnalyzeFile->GetDisplayName());
        mPreAnalyzeItemImage->SetTexture(mAnalyzeFile->GetIcon());
        break;

    case State::Map:
        AnalyzeMap_EnterState();
        break;

    case State::Image:
        AnalyzeImageEnter();
        break;
    }

    // The analyze button is clickable as long as we aren't in the "Empty" state.
    mAnalyzeButton->GetButton()->SetCanInteract(mState != State::Empty);
}

void SidneyAnalyze::SetStateFromFile()
{
    if(mAnalyzeFile == nullptr)
    {
        SetState(State::Empty);
    }
    else if(mAnalyzeFile->index == 19)
    {
        SetState(State::Map);
    }
    else if(mAnalyzeFile->type == SidneyFileType::Image)
    {
        SetState(State::Image);
    }
    else
    {
        printf("Unknown analyze state!\n");
    }
}

void SidneyAnalyze::OnAnalyzeButtonPressed()
{
    // We need a file to analyze.
    if(mAnalyzeFile == nullptr) { return; }

    // Set to right state depending on the file we're analyzing.
    SetStateFromFile();
    
    // Take the appropriate analyze action based on the item.
    switch(mState)
    {
    case State::Map:
        AnalyzeMap_OnAnalyzeButtonPressed();
        break;
    case State::Image:
        AnalyzeImage();
        break;
    }

    // This file has definitely been analyzed at least once now!
    mAnalyzeFile->hasBeenAnalyzed = true;
}

void SidneyAnalyze::ShowAnalyzeMessage(const std::string& message)
{
    mAnalyzeMessageWindow->SetActive(true);
    mAnalyzeMessage->SetText(SidneyUtil::GetAnalyzeLocalizer().GetText(message));
}

void SidneyAnalyze::AnalyzeImageInit()
{
    // Create a parent that contains all the image analysis stuff.
    mAnalyzeImageWindow = new Actor(TransformType::RectTransform);
    mAnalyzeImageWindow->GetTransform()->SetParent(mRoot->GetTransform());
    static_cast<RectTransform*>(mAnalyzeImageWindow->GetTransform())->SetAnchor(AnchorPreset::CenterStretch);
    static_cast<RectTransform*>(mAnalyzeImageWindow->GetTransform())->SetAnchoredPosition(0.0f, 0.0f);
    static_cast<RectTransform*>(mAnalyzeImageWindow->GetTransform())->SetSizeDelta(0.0f, 0.0f);

    // Create image that is being analyzed.
    {
        Actor* imageActor = new Actor(TransformType::RectTransform);
        imageActor->GetTransform()->SetParent(mAnalyzeImageWindow->GetTransform());

        mAnalyzeImage = imageActor->AddComponent<UIImage>();
        mAnalyzeImage->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
        mAnalyzeImage->GetRectTransform()->SetAnchoredPosition(10.0f, -50.0f);
    }

    // Hide by default.
    mAnalyzeImageWindow->SetActive(false);
}

void SidneyAnalyze::AnalyzeImageEnter()
{
    // Show the image view.
    mAnalyzeImageWindow->SetActive(true);

    // "Text" and "Graphic" dropdowns are available when analyzing an image. Map is not.
    mMenuBar.SetDropdownEnabled(1, true);
    mMenuBar.SetDropdownEnabled(2, true);
    mMenuBar.SetDropdownEnabled(3, false);

    // Show correct image and menu items based on current file.
    if(mAnalyzeFile->index == 20)
    {
        mAnalyzeImage->SetTexture(gAssetManager.LoadTexture("PARCHMENT1_BASE.BMP"), true);

        mMenuBar.SetDropdownChoiceEnabled(1, 0, true);
        mMenuBar.SetDropdownChoiceEnabled(1, 1, false);
        mMenuBar.SetDropdownChoiceEnabled(1, 2, false);
        mMenuBar.SetDropdownChoiceEnabled(1, 3, false);

        mMenuBar.SetDropdownChoiceEnabled(2, 0, true);
        mMenuBar.SetDropdownChoiceEnabled(2, 1, false);
        mMenuBar.SetDropdownChoiceEnabled(2, 2, false);
        mMenuBar.SetDropdownChoiceEnabled(2, 3, false);
        mMenuBar.SetDropdownChoiceEnabled(2, 4, false);
    }
    else if(mAnalyzeFile->index == 21)
    {
        mAnalyzeImage->SetTexture(gAssetManager.LoadTexture("PARCHMENT2_BASE.BMP"), true);

        mMenuBar.SetDropdownChoiceEnabled(1, 0, false);
        mMenuBar.SetDropdownChoiceEnabled(1, 1, false);
        mMenuBar.SetDropdownChoiceEnabled(1, 2, false);
        mMenuBar.SetDropdownChoiceEnabled(1, 3, true);

        mMenuBar.SetDropdownChoiceEnabled(2, 0, true);
        mMenuBar.SetDropdownChoiceEnabled(2, 1, true);
        mMenuBar.SetDropdownChoiceEnabled(2, 2, false);
        mMenuBar.SetDropdownChoiceEnabled(2, 3, false);
        mMenuBar.SetDropdownChoiceEnabled(2, 4, false);
    }
}

void SidneyAnalyze::AnalyzeImage()
{
    // Show correct analysis message depending on the file type.
    if(mAnalyzeFile->index == 20)
    {
        ShowAnalyzeMessage("AnalyzeParch1");
    }
    else if(mAnalyzeFile->index == 21)
    {
        ShowAnalyzeMessage("AnalyzeParch2");
    }
}