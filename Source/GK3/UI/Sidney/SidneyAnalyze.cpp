#include "SidneyAnalyze.h"

#include "ActionManager.h"
#include "AssetManager.h"
//#include "AudioManager.h"
#include "GameProgress.h"
#include "InventoryManager.h"
#include "SidneyButton.h"
#include "SidneyFiles.h"
#include "SidneyUtil.h"
//#include "Texture.h"
#include "UIButton.h"
#include "UICanvas.h"
#include "UINineSlice.h"
#include "UIImage.h"
//#include "UILabel.h"
#include "UILines.h"
#include "UIPoints.h"

namespace
{
    const float kZoomedOutMapSize = 342.0f;
    const float kZoomedInMapSize = 1368.0f;

    const float kZoomedInMapViewWidth = 271.0f;
    const float kZoomedInMapViewHeight = 324.0f;
}

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
    AnalyzeMapInit();
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
            printf("Use Shape\n");
            mSidneyFiles->ShowShapes([this](SidneyFile* selectedFile){
                printf("Chose %s\n", selectedFile->name.c_str());
            });
        });

        // (Note: "Save Shape" is Item6, but I don't think it is used in the final game?)

        // "Erase Shape" choice.
        mMenuBar.AddDropdownChoice(SidneyUtil::GetAnalyzeLocalizer().GetText("Menu3Item7"), [this](){
            printf("Erase Shape\n");
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
            AnalyzeMapSetStatusText(SidneyUtil::GetAnalyzeLocalizer().GetText("EnterPointsNote"));
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
        mAnalyzeMessage->SetText(SidneyUtil::GetAnalyzeLocalizer().GetText("MapNoPrimitiveNote"));
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
    AnalyzeMapUpdate(deltaTime);
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
        AnalyzeMapEnter();
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
        AnalyzeMap();
        break;
    case State::Image:
        AnalyzeImage();
        break;
    }

    // This file has definitely been analyzed at least once now!
    mAnalyzeFile->hasBeenAnalyzed = true;
}

void SidneyAnalyze::AnalyzeMapInit()
{
    // Create a parent "map window" object that contains all the map analysis stuff.
    mAnalyzeMapWindow = new Actor(TransformType::RectTransform);
    mAnalyzeMapWindow->GetTransform()->SetParent(mRoot->GetTransform());
    static_cast<RectTransform*>(mAnalyzeMapWindow->GetTransform())->SetAnchor(AnchorPreset::CenterStretch);
    static_cast<RectTransform*>(mAnalyzeMapWindow->GetTransform())->SetAnchoredPosition(0.0f, 0.0f);
    static_cast<RectTransform*>(mAnalyzeMapWindow->GetTransform())->SetSizeDelta(0.0f, 0.0f);

    // Create zoomed in map on left.
    {
        // Create an actor that represents the zoomed in window area.
        Actor* zoomedInMapWindow = new Actor(TransformType::RectTransform);
        zoomedInMapWindow->GetTransform()->SetParent(mAnalyzeMapWindow->GetTransform());

        mMap.zoomedIn.button = zoomedInMapWindow->AddComponent<UIButton>();

        // Put a canvas on it with masking! This allows us to move around the child map background and have it be masked outside the window area.
        UICanvas* zoomedInMapCanvas = zoomedInMapWindow->AddComponent<UICanvas>(-1);
        zoomedInMapCanvas->SetMasked(true);
        zoomedInMapCanvas->GetRectTransform()->SetAnchor(AnchorPreset::BottomLeft);
        zoomedInMapCanvas->GetRectTransform()->SetAnchoredPosition(9.0f, 73.0f);
        zoomedInMapCanvas->GetRectTransform()->SetSizeDelta(kZoomedInMapViewWidth, kZoomedInMapViewHeight);

        // Create big background image at full size.
        // Note that only a portion is visible due to the parent's canvas masking.
        Actor* zoomedInMapBackground = new Actor(TransformType::RectTransform);
        zoomedInMapBackground->GetTransform()->SetParent(zoomedInMapWindow->GetTransform());

        mMap.zoomedIn.background = zoomedInMapBackground->AddComponent<UIImage>();
        mMap.zoomedIn.background->SetTexture(gAssetManager.LoadTexture("SIDNEYBIGMAP.BMP"), true);
        mMap.zoomedIn.background->GetRectTransform()->SetAnchor(AnchorPreset::BottomLeft);

        // Create lines renderer.
        {
            Actor* linesActor = new Actor(TransformType::RectTransform);
            linesActor->GetTransform()->SetParent(zoomedInMapBackground->GetTransform());

            mMap.zoomedIn.lines = linesActor->AddComponent<UILines>();
            mMap.zoomedIn.lines->SetColor(Color32(0, 0, 0, 128));

            mMap.zoomedIn.lines->GetRectTransform()->SetAnchor(AnchorPreset::BottomLeft);
            mMap.zoomedIn.lines->GetRectTransform()->SetAnchoredPosition(Vector2::Zero);
        }

        // Create locked points renderer.
        {
            Actor* pointsActor = new Actor(TransformType::RectTransform);
            pointsActor->GetTransform()->SetParent(zoomedInMapBackground->GetTransform());

            mMap.zoomedIn.lockedPoints = pointsActor->AddComponent<UIPoints>();
            mMap.zoomedIn.lockedPoints->SetColor(Color32(0, 0, 255, 192));

            mMap.zoomedIn.lockedPoints->GetRectTransform()->SetAnchor(AnchorPreset::BottomLeft);
            mMap.zoomedIn.lockedPoints->GetRectTransform()->SetAnchoredPosition(Vector2::Zero);
        }

        // Create active points renderer.
        {
            Actor* pointsActor = new Actor(TransformType::RectTransform);
            pointsActor->GetTransform()->SetParent(zoomedInMapBackground->GetTransform());

            mMap.zoomedIn.points = pointsActor->AddComponent<UIPoints>();
            mMap.zoomedIn.points->SetColor(Color32(0, 255, 0, 192));

            mMap.zoomedIn.points->GetRectTransform()->SetAnchor(AnchorPreset::BottomLeft);
            mMap.zoomedIn.points->GetRectTransform()->SetAnchoredPosition(Vector2::Zero);
        }
    }

    // Create zoomed out map on right.
    {
        Actor* zoomedOutMapActor = new Actor(TransformType::RectTransform);
        zoomedOutMapActor->GetTransform()->SetParent(mAnalyzeMapWindow->GetTransform());

        mMap.zoomedOut.background = zoomedOutMapActor->AddComponent<UIImage>();
        mMap.zoomedOut.background->SetTexture(gAssetManager.LoadTexture("SIDNEYLITTLEMAP.BMP"));

        mMap.zoomedOut.background->GetRectTransform()->SetAnchor(AnchorPreset::BottomLeft);
        mMap.zoomedOut.background->GetRectTransform()->SetAnchoredPosition(289.0f, 55.0f);
        mMap.zoomedOut.background->GetRectTransform()->SetSizeDelta(kZoomedOutMapSize, kZoomedOutMapSize);

        mMap.zoomedOut.button = zoomedOutMapActor->AddComponent<UIButton>();

        // Create lines renderer.
        {
            Actor* linesActor = new Actor(TransformType::RectTransform);
            linesActor->GetTransform()->SetParent(zoomedOutMapActor->GetTransform());

            mMap.zoomedOut.lines = linesActor->AddComponent<UILines>();
            mMap.zoomedOut.lines->SetColor(Color32(0, 0, 0, 255));

            mMap.zoomedOut.lines->GetRectTransform()->SetAnchor(AnchorPreset::BottomLeft);
            mMap.zoomedOut.lines->GetRectTransform()->SetAnchoredPosition(Vector2::Zero);
        }

        // Create locked points renderer.
        {
            Actor* pointsActor = new Actor(TransformType::RectTransform);
            pointsActor->GetTransform()->SetParent(zoomedOutMapActor->GetTransform());

            mMap.zoomedOut.lockedPoints = pointsActor->AddComponent<UIPoints>();
            mMap.zoomedOut.lockedPoints->SetColor(Color32(0, 0, 255, 255));

            mMap.zoomedOut.lockedPoints->GetRectTransform()->SetAnchor(AnchorPreset::BottomLeft);
            mMap.zoomedOut.lockedPoints->GetRectTransform()->SetAnchoredPosition(Vector2::Zero);
        }

        // Create active points renderer.
        {
            Actor* pointsActor = new Actor(TransformType::RectTransform);
            pointsActor->GetTransform()->SetParent(zoomedOutMapActor->GetTransform());

            mMap.zoomedOut.points = pointsActor->AddComponent<UIPoints>();
            mMap.zoomedOut.points->SetColor(Color32(0, 255, 0, 255));

            mMap.zoomedOut.points->GetRectTransform()->SetAnchor(AnchorPreset::BottomLeft);
            mMap.zoomedOut.points->GetRectTransform()->SetAnchoredPosition(Vector2::Zero);
        }
    }

    // Create status text label.
    {
        Actor* statusTextActor = new Actor(TransformType::RectTransform);
        statusTextActor->GetTransform()->SetParent(mAnalyzeMapWindow->GetTransform());

        mMapStatusLabel = statusTextActor->AddComponent<UILabel>();
        mMapStatusLabel->SetFont(gAssetManager.LoadFont("SID_TEXT_14_GRN.FON"));
        mMapStatusLabel->SetText("Enter points on the map");

        mMapStatusLabel->GetRectTransform()->SetAnchor(AnchorPreset::BottomLeft);
        mMapStatusLabel->GetRectTransform()->SetAnchoredPosition(4.0f, 13.0f);
        mMapStatusLabel->SetEnabled(false);
    }

    // Hide by default.
    mAnalyzeMapWindow->SetActive(false);
}

void SidneyAnalyze::AnalyzeMapEnter()
{
    // Show the map view.
    mAnalyzeMapWindow->SetActive(true);

    // "Graphic" and "Map" dropdowns are available when analyzing a map. Text is not.
    mMenuBar.SetDropdownEnabled(1, false);
    mMenuBar.SetDropdownEnabled(2, true);
    mMenuBar.SetDropdownEnabled(3, true);

    // "Graphic" choices are mostly grayed out.
    mMenuBar.SetDropdownChoiceEnabled(2, 0, false);
    mMenuBar.SetDropdownChoiceEnabled(2, 1, false);
    mMenuBar.SetDropdownChoiceEnabled(2, 2, false);

    // Use & Erase Shape are enabled if we have any Shapes saved.
    mMenuBar.SetDropdownChoiceEnabled(2, 3, mSidneyFiles->HasFileOfType(SidneyFileType::Shape));
    mMenuBar.SetDropdownChoiceEnabled(2, 4, mSidneyFiles->HasFileOfType(SidneyFileType::Shape));
}

void SidneyAnalyze::AnalyzeMap()
{
    // Show the default/fallback message unless told otherwise.
    bool showFallbackAnalyzeMessage = true;
    
    // Main analyze window is opened, so user is asking us to analyze whatever they've placed on the map.
    bool workingOnAquarius = !gGameProgress.GetFlag("Aquarius");
    if(workingOnAquarius)
    {
        const Vector2 kRLCPoint(266.0f, 952.0f);
        const Vector2 kCDBPoint(653.0f, 1060.0f);
        const Vector2 kSunLineEndPoint(1336.0f, 1249.0f);

        // See if two placed points meet the criteria to finish Aquarius.
        Vector2 rlcPoint;
        Vector2 cdbPoint;
        for(size_t i = 0; i < mMap.zoomedIn.points->GetPointsCount(); ++i)
        {
            const Vector2& point = mMap.zoomedIn.points->GetPoint(i);
            if((point - kRLCPoint).GetLengthSq() < 20.0f * 20.0f)
            {
                rlcPoint = point;
            }
            else if((point - kCDBPoint).GetLengthSq() < 20.0f * 20.0f)
            {
                cdbPoint = point;
            }
        }

        if(rlcPoint != Vector2::Zero && cdbPoint != Vector2::Zero)
        {
            // Grace says "Cool!"
            gActionManager.ExecuteSheepAction("wait StartDialogue(\"02O3H2Z7F3\", 1)");

            // Show message confirming correct points.
            showFallbackAnalyzeMessage = false;
            mAnalyzeMessage->SetText(SidneyUtil::GetAnalyzeLocalizer().GetText("MapLine1Note"));
            mAnalyzeMessageWindow->SetActive(true);

            // Correct points move from "active" points to "locked" points.
            mMap.zoomedIn.points->RemovePoint(rlcPoint);
            mMap.zoomedIn.lockedPoints->AddPoint(kRLCPoint);

            mMap.zoomedIn.points->RemovePoint(cdbPoint);
            mMap.zoomedIn.lockedPoints->AddPoint(kCDBPoint);

            mMap.zoomedOut.points->RemovePoint(mMap.ZoomedInToZoomedOutPos(rlcPoint));
            mMap.zoomedOut.lockedPoints->AddPoint(mMap.ZoomedInToZoomedOutPos(kRLCPoint));

            mMap.zoomedOut.points->RemovePoint(mMap.ZoomedInToZoomedOutPos(cdbPoint));
            mMap.zoomedOut.lockedPoints->AddPoint(mMap.ZoomedInToZoomedOutPos(kCDBPoint));

            // Add sun line through the placed points.
            mMap.zoomedIn.lines->AddLine(kRLCPoint, kSunLineEndPoint);
            mMap.zoomedOut.lines->AddLine(mMap.ZoomedInToZoomedOutPos(kRLCPoint),
                                            mMap.ZoomedInToZoomedOutPos(kSunLineEndPoint));

            // Done with Aquarius!
            gGameProgress.SetFlag("Aquarius");
        }
        else if(mMap.zoomedIn.points->GetPointsCount() > 0)
        {
            showFallbackAnalyzeMessage = false;
            mAnalyzeMessage->SetText(SidneyUtil::GetAnalyzeLocalizer().GetText("MapIndeterminateNote"));
            mAnalyzeMessageWindow->SetActive(true);
        }
    }

    // If you analyze the map, but there is not a more specific message to show, we always show this fallback at least.
    if(showFallbackAnalyzeMessage)
    {
        // If points are placed, the message is different.
        if(mMap.zoomedIn.points->GetPointsCount() > 0)
        {
            // Says "unclear how to analyze those points."
            mAnalyzeMessage->SetText(SidneyUtil::GetAnalyzeLocalizer().GetText("MapIndeterminateNote"));
            mAnalyzeMessageWindow->SetActive(true);
        }
        else
        {
            // Says "map is too complex to analyze - try adding points or shapes."
            mAnalyzeMessage->SetText(SidneyUtil::GetAnalyzeLocalizer().GetText("MapNoPrimitiveNote"));
            mAnalyzeMessageWindow->SetActive(true);
        }
    }
}

void SidneyAnalyze::AnalyzeMapUpdate(float deltaTime)
{
    if(!mAnalyzeMapWindow->IsActive()) { return; }

    // Clicking on the zoomed out map should focus that spot on the zoomed in map.
    if(mMap.zoomedOut.button->IsHovered() && gInputManager.IsMouseButtonPressed(InputManager::MouseButton::Left))
    {
        Vector2 zoomedOutMapPos = mMap.zoomedOut.GetLocalPosFromMousePos();
        //printf("%f, %f\n", zoomedOutMapPos.x, zoomedOutMapPos.y);

        Vector2 zoomedInMapPos = mMap.ZoomedOutToZoomedInPos(zoomedOutMapPos);
        //printf("%f, %f\n", zoomedInPos.x, zoomedInPos.y);

        // The zoomed in view should center on the zoomed in pos.
        Vector2 halfViewSize(kZoomedInMapViewWidth * 0.5f, kZoomedInMapViewHeight * 0.5f);
        Rect zoomedInRect(zoomedInMapPos - halfViewSize, zoomedInMapPos + halfViewSize);

        // Make sure the zoomed in rect is within the bounds of the map.
        Vector2 zoomedInRectMin = zoomedInRect.GetMin();
        Vector2 zoomedInRectMax = zoomedInRect.GetMax();
        if(zoomedInRectMin.x < 0)
        {
            zoomedInRect.x += Math::Abs(zoomedInRectMin.x);
        }
        if(zoomedInRectMin.y < 0)
        {
            zoomedInRect.y += Math::Abs(zoomedInRectMin.y);
        }
        if(zoomedInRectMax.x >= kZoomedInMapSize)
        {
            zoomedInRect.x -= (zoomedInRectMax.x - kZoomedInMapSize);
        }
        if(zoomedInRectMax.y >= kZoomedInMapSize)
        {
            zoomedInRect.y -= (zoomedInRectMax.y - kZoomedInMapSize);
        }

        // Position the map based on the zoomed in rect.
        mMap.zoomedIn.background->GetRectTransform()->SetAnchoredPosition(-zoomedInRect.GetMin());
    }

    // Clicking on the zoomed in map may perform an action in some cases.
    if(mMap.zoomedIn.button->IsHovered() && gInputManager.IsMouseButtonLeadingEdge(InputManager::MouseButton::Left))
    {
        if(mEnteringPoints)
        {
            // Add point to zoomed in map at click pos.
            Vector2 zoomedInMapPos = mMap.zoomedIn.GetLocalPosFromMousePos();
            mMap.zoomedIn.points->AddPoint(zoomedInMapPos);
            printf("Add pt at %f, %f\n", zoomedInMapPos.x, zoomedInMapPos.y);

            // Also convert to zoomed out map position and add point there.
            mMap.zoomedOut.points->AddPoint(mMap.ZoomedInToZoomedOutPos(zoomedInMapPos));
        }
    }

    // Hide map status label if enough time has passed.
    if(mMapStatusLabelTimer > 0.0f)
    {
        mMapStatusLabelTimer -= deltaTime;
        if(mMapStatusLabelTimer <= 0.0f)
        {
            mMapStatusLabel->SetEnabled(false);
        }
    }
}

void SidneyAnalyze::AnalyzeMapSetStatusText(const std::string& text)
{
    mMapStatusLabel->SetText(text);
    mMapStatusLabel->SetEnabled(true);
    mMapStatusLabelTimer = 5.0f;
}

Vector2 SidneyAnalyze::MapState::UI::GetLocalPosFromMousePos()
{
    // Get background's min position (lower left corner).
    Vector2 min = background->GetRectTransform()->GetWorldRect().GetMin();

    // Subtract from mouse pos to get point relative to lower left corner.
    return gInputManager.GetMousePosition() - min;
}

Vector2 SidneyAnalyze::MapState::ZoomedOutToZoomedInPos(const Vector2& pos)
{
    // Transform point from zoomed out map coordinate to zoomed in map coordinate.
    Vector2 zoomedOutPosNorm = pos / kZoomedOutMapSize;
    return zoomedOutPosNorm * kZoomedInMapSize;
}

Vector2 SidneyAnalyze::MapState::ZoomedInToZoomedOutPos(const Vector2& pos)
{
    Vector2 zoomedInPosNorm = pos / kZoomedInMapSize;
    return zoomedInPosNorm * kZoomedOutMapSize;
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
    mAnalyzeMessageWindow->SetActive(true);

    // Show correct analysis message depending on the file type.
    if(mAnalyzeFile->index == 20)
    {
        mAnalyzeMessage->SetText(SidneyUtil::GetAnalyzeLocalizer().GetText("AnalyzeParch1"));
    }
    else if(mAnalyzeFile->index == 21)
    {
        mAnalyzeMessage->SetText(SidneyUtil::GetAnalyzeLocalizer().GetText("AnalyzeParch2"));
    }
}