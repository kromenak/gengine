#include "SidneyAnalyze.h"

#include "Actor.h"
#include "ActionManager.h"
#include "AssetManager.h"
#include "AudioManager.h"
#include "GameProgress.h"
#include "InputManager.h"
#include "LocationManager.h"
#include "RectTransform.h"
#include "Sidney.h"
#include "SidneyFakeInputPopup.h"
#include "SidneyFiles.h"
#include "SidneyUtil.h"
#include "UIButton.h"
#include "UICircles.h"
#include "UIGrids.h"
#include "UIHexagrams.h"
#include "UILabel.h"
#include "UILines.h"
#include "UIPoints.h"
#include "UIRectangles.h"
#include "UIUtil.h"

namespace
{
    // The distance from the mouse pointer to a point for that point to be considered "hovered."
    const float kHoverPointDist = 4.0f;
    const float kHoverPointDistSq = kHoverPointDist * kHoverPointDist;

    // Position of Rennes-les-Chateau in zoomed-in map coordinates.
    const Vector2 kRLCPoint(267.0f, 953.0f);

    // Points required for Pisces (in zoomed in map coordinates).
    const Vector2 kPiscesCoustaussaPoint(404.0f, 1095.0f);
    const Vector2 kPiscesBezuPoint(301.0f, 386.0f);
    const Vector2 kPiscesBugarachPoint(990.0f, 326.0f);

    // The center point and radius of the circle placed on the map (in zoomed out map coordinates).
    // This is also the center point and radius used for the square around the circle, and the hexagram placed in the circle.
    const Vector2 kCircleCenter(169.0f, 174.0f);
    const float kCircleRadius = 121.0f;

    // For placing the square around the circle, it's size and correct rotation in radians.
    const float kSquareAroundCircleSize = (kCircleRadius * 2) + 1;
    const float kSquareAroundCircleRotationRadians = 1.185174f;

    // Meridian line points for taurus (in zoomed in map coordinates).
    const Vector2 kTaurusSerresPoint(808.0f, 1168.0f);
    const Vector2 kTaurusMeridianPoint(896.0f, 1130.0f);

    // For placing the hexagram, the expected rotation in degrees.
    const float kHexagramRotationDegrees = 33.0f;

    // The four corners of the temple (in zoomed in map coordinates).
    const Vector2 kTempleCorner1(381.0f, 292.0f);
    const Vector2 kTempleCorner2(605.0f, 201.0f);
    const Vector2 kTempleCorner3(970.0f, 1102.0f);
    const Vector2 kTempleCorner4(745.0f, 1193.0f);

    // The four division points of the temple (in zoomed in map coordinates).
    const Vector2 kTempleDivisionPoint1(654.0f, 967.0f);
    const Vector2 kTempleDivisionPoint2(879.0f, 877.0f);
    const Vector2 kTempleDivisionPoint3(471.0f, 518.0f);
    const Vector2 kTempleDivisionPoint4(695.0f, 426.0f);

    // "The Site" point on the map (in zoomed in map coordinates);
    const Vector2 kTheSitePoint(812.0f, 1033.0f);
}

void SidneyAnalyze::AnalyzeMap_Init()
{
    // Create a parent "map window" object that contains all the map analysis stuff.
    mAnalyzeMapWindow = new Actor("Analyze Map", TransformType::RectTransform);
    mAnalyzeMapWindow->GetTransform()->SetParent(mRoot->GetTransform());
    static_cast<RectTransform*>(mAnalyzeMapWindow->GetTransform())->SetAnchor(AnchorPreset::CenterStretch);
    static_cast<RectTransform*>(mAnalyzeMapWindow->GetTransform())->SetSizeDelta(0.0f, 0.0f);

    // Init the map, which builds the UI widgets for both the zoomed in and zoomed out maps.
    mMap.Init(mAnalyzeMapWindow);

    // Create status text label.
    {
        mMapStatusLabel = UI::CreateWidgetActor<UILabel>("MapStatus", mAnalyzeMapWindow);
        mMapStatusLabel->SetFont(gAssetManager.LoadFont("SID_TEXT_14_GRN.FON"));
        mMapStatusLabel->GetRectTransform()->SetAnchor(AnchorPreset::BottomLeft);
        mMapStatusLabel->GetRectTransform()->SetAnchoredPosition(4.0f, 13.0f);
        mMapStatusLabel->SetEnabled(false);
    }

    // Hide by default.
    mAnalyzeMapWindow->SetActive(false);
}

void SidneyAnalyze::AnalyzeMap_EnterState()
{
    // Show the map view.
    mAnalyzeMapWindow->SetActive(true);

    // "Graphic" and "Map" dropdowns are available when analyzing a map. Text is not.
    mMenuBar.SetDropdownEnabled(kTextDropdownIdx, false);
    mMenuBar.SetDropdownEnabled(kGraphicDropdownIdx, true);
    mMenuBar.SetDropdownEnabled(kMapDropdownIdx, true);

    // "Graphic" choices are mostly grayed out.
    mMenuBar.SetDropdownChoiceEnabled(kGraphicDropdownIdx, kGraphicDropdown_ViewGeometryIdx, false);
    mMenuBar.SetDropdownChoiceEnabled(kGraphicDropdownIdx, kGraphicDropdown_RotateShapeIdx, false);
    mMenuBar.SetDropdownChoiceEnabled(kGraphicDropdownIdx, kGraphicDropdown_ZoomClarifyIdx, false);

    // Use & Erase Shape are enabled if we have any Shapes saved.
    mMenuBar.SetDropdownChoiceEnabled(kGraphicDropdownIdx, kGraphicDropdown_UseShapeIdx, mSidneyFiles->HasFileOfType(SidneyFileType::Shape));
    mMenuBar.SetDropdownChoiceEnabled(kGraphicDropdownIdx, kGraphicDropdown_EraseShapeIdx, mSidneyFiles->HasFileOfType(SidneyFileType::Shape));

    // Refresh site label visibility on map.
    mMap.RefreshImages();
}

void SidneyAnalyze::AnalyzeMap_Update(float deltaTime)
{
    if(!mAnalyzeMapWindow->IsActive()) { return; }

    // Hide map status label if enough time has passed.
    if(mMapStatusLabelTimer > 0.0f)
    {
        mMapStatusLabelTimer -= deltaTime;
        if(mMapStatusLabelTimer <= 0.0f)
        {
            mMapStatusLabel->SetEnabled(false);
        }
    }

    // Do not update maps or LSR progress if an action is active.
    if(gActionManager.IsActionPlaying())
    {
        mMenuBar.SetInteractive(false);
        return;
    }

    // Do not update maps or LSR progress when the analyze message is visible.
    if(mAnalyzePopup->IsActive())
    {
        mMenuBar.SetInteractive(false);
        return;
    }
    mMenuBar.SetInteractive(true);

    // Update interaction with the zoomed out map.
    AnalyzeMap_UpdateZoomedOutMap(deltaTime);

    // Update interaction with the zoomed in map.
    AnalyzeMap_UpdateZoomedInMap(deltaTime);

    // To complete Pisces, you must place three points on the map AND align a circle to them.
    bool aquariusDone = gGameProgress.GetFlag("Aquarius");
    bool piscesDone = gGameProgress.GetFlag("Pisces");
    bool ariesDone = gGameProgress.GetFlag("Aries");
    bool taurusDone = gGameProgress.GetFlag("Taurus");
    bool leoDone = gGameProgress.GetFlag("Leo");
    bool virgoDone = gGameProgress.GetFlag("Virgo");
    bool libraDone = gGameProgress.GetFlag("Libra");
    if(aquariusDone && !piscesDone)
    {
        AnalyzeMap_CheckPiscesCompletion();
    }
    else if(piscesDone && !ariesDone)
    {
        AnalyzeMap_CheckAriesCompletion();
    }
    else if(ariesDone && !taurusDone)
    {
        AnalyzeMap_CheckTaurusCompletion();
    }
    else if(leoDone && virgoDone && !libraDone) // both Leo and Virgo must be done before you can complete Libra
    {
        AnalyzeMap_CheckLibraCompletion();
    }
}

void SidneyAnalyze::AnalyzeMap_UpdateZoomedOutMap(float deltaTime)
{
    // If mouse button is not pressed, clear click action.
    if(!gInputManager.IsMouseButtonPressed(InputManager::MouseButton::Left))
    {
        mMap.zoomedOutClickAction = MapState::ClickAction::None;
    }

    // If we're hovering the zoomed out map, we may be trying to interact with it.
    if(mMap.zoomedOut.button->IsHovered())
    {
        Vector2 zoomedOutMapPos = mMap.zoomedOut.GetLocalMousePos();
        //printf("%f, %f\n", zoomedOutMapPos.x, zoomedOutMapPos.y);

        // If hovering a point (locked or not) with no selected shape, update map status text with that point's lat/long.
        if(!mMap.IsAnyShapeSelected())
        {
            bool setText = false;
            for(size_t i = 0; i < mMap.zoomedOut.points->GetCount(); ++i)
            {
                const Vector2& point = mMap.zoomedOut.points->Get(i);
                float distToPointSq = (zoomedOutMapPos - point).GetLengthSq();
                if(distToPointSq < kHoverPointDistSq)
                {
                    AnalyzeMap_SetPointStatusText("MapHoverPointNote", mMap.ToZoomedInPoint(point));
                    setText = true;
                    break;
                }
            }
            if(!setText)
            {
                for(size_t i = 0; i < mMap.zoomedOut.lockedPoints->GetCount(); ++i)
                {
                    const Vector2& point = mMap.zoomedOut.lockedPoints->Get(i);
                    float distToPointSq = (zoomedOutMapPos - point).GetLengthSq();
                    if(distToPointSq < kHoverPointDistSq)
                    {
                        AnalyzeMap_SetPointStatusText("MapHoverPointNote", mMap.ToZoomedInPoint(point));
                        setText = true;
                        break;
                    }
                }
            }
        }

        // On the zoomed out map, you can select shapes, move them, resize them, and rotate them.
        // This update does all the heavy lifting of detecting and performing shape manipulations like that.
        mMap.UpdateZoomedOutShapeManipulation();
    }
}

void SidneyAnalyze::AnalyzeMap_UpdateZoomedInMap(float deltaTime)
{
    // Clicking on the zoomed in map may perform an action in some cases.
    if(mMap.zoomedIn.button->IsHovered() && gInputManager.IsMouseButtonLeadingEdge(InputManager::MouseButton::Left))
    {
        if(mMap.enteringPoints)
        {
            // Add point to zoomed in map at click pos.
            Vector2 zoomedInMapPos = mMap.zoomedIn.GetLocalMousePos();
            mMap.zoomedIn.points->Add(zoomedInMapPos);
            printf("Add pt at %f, %f\n", zoomedInMapPos.x, zoomedInMapPos.y);

            // Also convert to zoomed out map position and add point there.
            mMap.zoomedOut.points->Add(mMap.ToZoomedOutPoint(zoomedInMapPos));

            // When you place a point, the latitude/longitude of the point are displayed on-screen.
            AnalyzeMap_SetPointStatusText("MapEnterPointNote", zoomedInMapPos);

            // There's one part (during Scorpio) when entering a point immediately illicits a response from Grace.
            if(gGameProgress.GetFlag("Libra") && !gGameProgress.GetFlag("Scorpio") && gGameProgress.GetFlag("PlacedTempleDivisions"))
            {
                AnalyzeMap_CheckScorpioCompletion(zoomedInMapPos);
            }
        }
    }
}

void SidneyAnalyze::AnalyzeMap_OnAnalyzeButtonPressed()
{
    bool didValidAnalyzeAction = false;

    // The map analysis behavior depends on what part of the LSR riddle we're on.
    bool aquariusDone = gGameProgress.GetFlag("Aquarius");
    bool piscesDone = gGameProgress.GetFlag("Pisces");
    bool ariesDone = gGameProgress.GetFlag("Aries");
    bool taurusDone = gGameProgress.GetFlag("Taurus");
    bool geminiDone = gGameProgress.GetFlag("Gemini");
    bool cancerDone = gGameProgress.GetFlag("Cancer");
    bool leoDone = gGameProgress.GetFlag("Leo");
    bool virgoDone = gGameProgress.GetFlag("Virgo");
    bool libraDone = gGameProgress.GetFlag("Libra");
    bool scorpioDone = gGameProgress.GetFlag("Scorpio");
    bool sagitariusDone = gGameProgress.GetFlag("Sagittarius");
    if(!aquariusDone) // Working on Aquarius
    {
        AnalyzeMap_CheckAquariusCompletion();
    }
    else if(aquariusDone && !piscesDone) // Working on Pisces
    {
        // Find placed points that are close enough to the desired points.
        Vector2 coustaussaPoint = mMap.zoomedIn.GetPlacedPointNearPoint(kPiscesCoustaussaPoint);
        Vector2 bezuPoint = mMap.zoomedIn.GetPlacedPointNearPoint(kPiscesBezuPoint);
        Vector2 bugarachPoint = mMap.zoomedIn.GetPlacedPointNearPoint(kPiscesBugarachPoint);

        // If you place all the points, you get a different analysis message.
        // But you still need to place the Circle shape to solve Pisces!
        if(coustaussaPoint != Vector2::Zero && bezuPoint != Vector2::Zero && bugarachPoint != Vector2::Zero)
        {
            // Says "several possible linkages - use shapes or more points."
            ShowAnalyzeMessage("MapSeveralPossNote");
        }
    }
    else if((piscesDone && !ariesDone) || (ariesDone && !taurusDone)) // Working on Aries OR Taurus
    {
        AnalyzeMap_CheckTaurusMeridianLine();
    }
    else if(geminiDone && cancerDone && (!leoDone || !virgoDone)) // working on Leo or Virgo
    {
        // Players are logically supposed to complete Leo before Virgo, but they can be done in either order.

        // There's also the scenario to cover where the player placed the points for BOTH of these at the same time (tricky tricky).
        // In that case, Leo should complete first, and then Virgo on another button press.

        // First, checkif leo was completed, if not already done.
        bool justCompletedLeo = false;
        if(!leoDone)
        {
            justCompletedLeo = AnalyzeMap_CheckLeoCompletion();
        }

        // If we didn't finish Leo (or it was already done), check if we finished Virgo.
        if(!virgoDone && !justCompletedLeo)
        {
            AnalyzeMap_CheckVirgoCompletion();
        }
    }
    else if(libraDone && !scorpioDone) // working on Scorpio
    {
        didValidAnalyzeAction = AnalyzeMap_CheckScorpioPlaceTempleDivisions();
    }

    // Sagitarius functions a bit differently from other LSR steps, in that you can try to do it at any time, and the game does give a response if it's the wrong time for it.
    // (I'm not sure if this was a bug in the original game, but let's mimic that behavior).
    if(!didValidAnalyzeAction && !sagitariusDone)
    {
        didValidAnalyzeAction = AnalyzeMap_CheckSagitariusCompletion();
    }

    // If a popup was displayed, then we must have performed a valid/recognized analyze action.
    if(mAnalyzePopup->IsActive())
    {
        didValidAnalyzeAction = true;
    }

    // If you analyze the map, but there is not a more specific message to show, we always show this fallback at least.
    if(!didValidAnalyzeAction)
    {
        // If points are placed, the message is different.
        if(mMap.zoomedIn.points->GetCount() > 0 || mMap.zoomedIn.lockedPoints->GetCount() > 0)
        {
            // Says "unclear how to analyze those points."
            ShowAnalyzeMessage("MapIndeterminateNote", Vector2(), HorizontalAlignment::Center);
        }
        else
        {
            // Says "map is too complex to analyze - try adding points or shapes."
            ShowAnalyzeMessage("MapNoPrimitiveNote");
        }
    }

    // Analyzing always clears point entry and shape selection.
    mMap.enteringPoints = false;
    mMap.ClearShapeSelection();
}

void SidneyAnalyze::AnalyzeMap_OnUseShapePressed()
{
    mSidneyFiles->ShowShapes([this](SidneyFile* selectedFile){
        mMap.AddShape(selectedFile->name);
    });
}

void SidneyAnalyze::AnalyzeMap_OnEraseShapePressed()
{
    if(mMap.IsAnyShapeSelected())
    {
        // After completing Aries, the player has placed a Rectangle of the right size on the map, but it still needs to be rotated for Taurus.
        // At this point, the game stops you from erasing the Rectangle, since that would "uncomplete" Aries.
        bool ariesCompleted = gGameProgress.GetFlag("Aries");
        bool taurusCompleted = gGameProgress.GetFlag("Taurus");
        if(mMap.selectedRectangleIndex >= 0 && (ariesCompleted && !taurusCompleted))
        {
            // Grace says "I think that's right - I don't want to erase it."
            gActionManager.ExecuteSheepAction("wait StartDialogue(\"02O0I27LN1\", 1)");
        }
        else
        {
            mMap.EraseSelectedShape();
        }
    }
    else
    {
        ShowAnalyzeMessage("NoShapeNote", Vector2(), HorizontalAlignment::Center);
    }
}

void SidneyAnalyze::AnalyzeMap_OnEnterPointsPressed()
{
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
    mMap.enteringPoints = true;
}

void SidneyAnalyze::AnalyzeMap_OnClearPointsPressed()
{
    mMap.enteringPoints = false;
    mMap.zoomedIn.points->Clear();
    mMap.zoomedOut.points->Clear();
}

void SidneyAnalyze::AnalyzeMap_OnDrawGridPressed()
{
    if(mMap.zoomedIn.grids->GetCount() == 0)
    {
        // Figure out what we want the grid to fill (whole screen or a shape).
        std::vector<std::string> gridFillChoices = {
            SidneyUtil::GetAnalyzeLocalizer().GetText("GridFillScreen"),
            SidneyUtil::GetAnalyzeLocalizer().GetText("GridFillShape"),
            SidneyUtil::GetAnalyzeLocalizer().GetText("GridCancel")
        };
        mSidneyFiles->ShowCustom(SidneyUtil::GetAnalyzeLocalizer().GetText("GridList"), gridFillChoices, [this](size_t fillChoiceIdx){
            // Early out if user chose "cancel" option.
            if(fillChoiceIdx == 2) { return; }

            // Determine whether we want to fill the screen or a shape.
            bool fillShape = (fillChoiceIdx == 1);

            // Grace usually doesn't want to fill a shape.
            // There's only one time (during Gemini) that she'll let you proceed past this point.
            if(fillShape)
            {
                bool taurusDone = gGameProgress.GetFlag("Taurus");
                bool geminiDone = gGameProgress.GetFlag("Gemini");
                bool workingOnGemini = (taurusDone && !geminiDone);
                if(!workingOnGemini)
                {
                    // "I don't think a grid will help me here."
                    gActionManager.ExecuteSheepAction("wait StartDialogue(\"02OD32ZNF1\", 1)");
                    return;
                }
            }

            // Figure out what size grid to use.
            std::vector<std::string> gridSizeChoices = {
                SidneyUtil::GetAnalyzeLocalizer().GetText("Grid2"),
                SidneyUtil::GetAnalyzeLocalizer().GetText("Grid4"),
                SidneyUtil::GetAnalyzeLocalizer().GetText("Grid8"),
                SidneyUtil::GetAnalyzeLocalizer().GetText("Grid12"),
                SidneyUtil::GetAnalyzeLocalizer().GetText("Grid16"),
                SidneyUtil::GetAnalyzeLocalizer().GetText("GridCancel")
            };
            mSidneyFiles->ShowCustom(SidneyUtil::GetAnalyzeLocalizer().GetText("GridList"), gridSizeChoices, [this, fillShape](size_t sizeChoiceIdx){
                // Early out if user chose "cancel" option.
                if(sizeChoiceIdx == 5) { return; }

                // Otherwise, figure out size.
                uint8_t gridSize = 2;
                switch(sizeChoiceIdx)
                {
                case 0:
                    gridSize = 2;
                    break;
                case 1:
                    gridSize = 4;
                    break;
                case 2:
                    gridSize = 8;
                    break;
                case 3:
                    gridSize = 12;
                    break;
                case 4:
                    gridSize = 16;
                    break;
                }

                // Draw the grid.
                mMap.DrawGrid(gridSize, fillShape);

                // Usually, drawing a grid is NOT the answer!
                // In fact, the only time it actually helps is during Gemini if you fill the shape.
                bool taurusDone = gGameProgress.GetFlag("Taurus");
                bool geminiDone = gGameProgress.GetFlag("Gemini");
                bool workingOnGemini = (taurusDone && !geminiDone);
                if(workingOnGemini && fillShape)
                {
                    AnalyzeMap_CheckGeminiAndCancerCompletion(gridSize);
                }
                else
                {
                    // Grace says "Hmm, not sure about that."
                    gActionManager.ExecuteSheepAction("wait StartDialogue(\"02O0I27GT1\", 1)");
                }
            });
        });
    }
    else
    {
        // Only allowed to place one grid at a time. This message says "erase other grid first."
        ShowAnalyzeMessage("GridDispNote");
    }
}

void SidneyAnalyze::AnalyzeMap_OnEraseGridPressed()
{
    if(mMap.zoomedIn.grids->GetCount() > 0)
    {
        mMap.ClearGrid();
    }
    else
    {
        // Display an error if there is no grid to erase.
        ShowAnalyzeMessage("NoGridEraseNote");
    }
}

void SidneyAnalyze::AnalyzeMap_SetStatusText(const std::string& text, float duration)
{
    mMapStatusLabel->SetEnabled(true);
    mMapStatusLabel->SetText(text);
    mMapStatusLabelTimer = duration;
}

void SidneyAnalyze::AnalyzeMap_SetPointStatusText(const std::string& baseMessage, const Vector2& zoomedInMapPos)
{
    AnalyzeMap_SetStatusText(StringUtil::Format(SidneyUtil::GetAnalyzeLocalizer().GetText(baseMessage).c_str(),
                                                mMap.GetPointLatLongText(zoomedInMapPos).c_str()), 0.0f);
}

void SidneyAnalyze::AnalyzeMap_CheckAquariusCompletion()
{
    // Player must place two points near enough to these points and press "Analyze" to pass Aquarius.
    const Vector2 kCDBPoint(652.0f, 1061.0f);
    const Vector2 kSunLineEndPoint(1336.0f, 1247.0f);

    // See if two placed points meet the criteria to finish Aquarius.
    Vector2 rlcPoint = mMap.zoomedIn.GetPlacedPointNearPoint(kRLCPoint);
    Vector2 cdbPoint = mMap.zoomedIn.GetPlacedPointNearPoint(kCDBPoint);
    if(rlcPoint != Vector2::Zero && cdbPoint != Vector2::Zero)
    {
        // Grace says "Cool!"
        gActionManager.ExecuteSheepAction("wait StartDialogue(\"02O3H2Z7F3\", 1)");

        // Show message confirming correct points.
        ShowAnalyzeMessage("MapLine1Note");

        // Correct points move from "active" points to "locked" points.
        mMap.zoomedIn.points->Remove(rlcPoint);
        mMap.zoomedIn.lockedPoints->Add(kRLCPoint);

        mMap.zoomedIn.points->Remove(cdbPoint);
        mMap.zoomedIn.lockedPoints->Add(kCDBPoint);

        mMap.zoomedOut.points->Remove(mMap.ToZoomedOutPoint(rlcPoint));
        mMap.zoomedOut.lockedPoints->Add(mMap.ToZoomedOutPoint(kRLCPoint));

        mMap.zoomedOut.points->Remove(mMap.ToZoomedOutPoint(cdbPoint));
        mMap.zoomedOut.lockedPoints->Add(mMap.ToZoomedOutPoint(kCDBPoint));

        // Add sun line through the placed points.
        mMap.zoomedIn.lines->Add(kRLCPoint, kSunLineEndPoint);
        mMap.zoomedOut.lines->Add(mMap.ToZoomedOutPoint(kRLCPoint),
                                      mMap.ToZoomedOutPoint(kSunLineEndPoint));

        // Done with Aquarius!
        gGameProgress.SetFlag("Aquarius");
        gGameProgress.SetFlag("PlacedSunriseLine");
        gGameProgress.ChangeScore("e_sidney_map_aquarius");
        SidneyUtil::UpdateLSRState();
    }
}

void SidneyAnalyze::AnalyzeMap_CheckPiscesCompletion()
{
    Vector2 coustaussaPoint = mMap.zoomedIn.GetPlacedPointNearPoint(kPiscesCoustaussaPoint);
    Vector2 bezuPoint = mMap.zoomedIn.GetPlacedPointNearPoint(kPiscesBezuPoint);
    Vector2 bugarachPoint = mMap.zoomedIn.GetPlacedPointNearPoint(kPiscesBugarachPoint);
    if(coustaussaPoint != Vector2::Zero && bezuPoint != Vector2::Zero && bugarachPoint != Vector2::Zero)
    {
        for(size_t i = 0; i < mMap.zoomedOut.circles->GetCount(); ++i)
        {
            const Circle& circle = mMap.zoomedOut.circles->Get(i);

            float centerDiffSq = (circle.center - kCircleCenter).GetLengthSq();
            float radiusDiff = Math::Abs(circle.radius - kCircleRadius);
            if(centerDiffSq < 20 * 20 && radiusDiff < 4)
            {
                // Put locked points on the zoomed in map.
                mMap.zoomedIn.points->Remove(coustaussaPoint);
                mMap.zoomedIn.points->Remove(bezuPoint);
                mMap.zoomedIn.points->Remove(bugarachPoint);

                mMap.zoomedIn.lockedPoints->Add(kPiscesCoustaussaPoint);
                mMap.zoomedIn.lockedPoints->Add(kPiscesBezuPoint);
                mMap.zoomedIn.lockedPoints->Add(kPiscesBugarachPoint);

                // Same for the zoomed out map.
                mMap.zoomedOut.points->Remove(mMap.ToZoomedOutPoint(coustaussaPoint));
                mMap.zoomedOut.points->Remove(mMap.ToZoomedOutPoint(bezuPoint));
                mMap.zoomedOut.points->Remove(mMap.ToZoomedOutPoint(bugarachPoint));

                mMap.zoomedOut.lockedPoints->Add(mMap.ToZoomedOutPoint(kPiscesCoustaussaPoint));
                mMap.zoomedOut.lockedPoints->Add(mMap.ToZoomedOutPoint(kPiscesBezuPoint));
                mMap.zoomedOut.lockedPoints->Add(mMap.ToZoomedOutPoint(kPiscesBugarachPoint));

                // As part of Pisces, the player *may* have also places a point on RLC again.
                // If so, we can get rid of it now, to keep the map a bit cleaner.
                Vector2 rlcPoint = mMap.zoomedIn.GetPlacedPointNearPoint(kRLCPoint);
                if(rlcPoint != Vector2::Zero)
                {
                    mMap.zoomedIn.points->Remove(rlcPoint);
                    mMap.zoomedOut.points->Remove(mMap.ToZoomedOutPoint(rlcPoint));
                }

                // Put locked circle on zoomed out map.
                mMap.zoomedOut.circles->Clear();
                mMap.zoomedIn.circles->Clear();
                mMap.selectedCircleIndex = -1;

                mMap.zoomedOut.lockedCircles->Add(kCircleCenter, kCircleRadius);
                mMap.zoomedIn.lockedCircles->Add(mMap.ToZoomedInPoint(kCircleCenter), mMap.ToZoomedInDistance(kCircleRadius));

                // Grace is excited that we figured it out.
                gActionManager.ExecuteSheepAction("wait StartDialogue(\"02OAG2ZJU2\", 2)", [](const Action* action){
                    gAudioManager.PlaySFX(gAssetManager.LoadAudio("CLOCKTIMEBLOCK.WAV"));
                });

                // Show confirmation message.
                std::string message = StringUtil::Format(SidneyUtil::GetAnalyzeLocalizer().GetText("MapCircleConfirmNote").c_str(),
                                                         mMap.GetPointLatLongText(mMap.ToZoomedInPoint(kCircleCenter)).c_str());
                ShowAnalyzeMessage(message);

                // This also gets us the coordinates at the center of the circle as an inventory item.
                gInventoryManager.AddInventoryItem("GRACE_COORDINATE_PAPER_1");

                // We completed Pisces.
                gGameProgress.SetFlag("Pisces");
                gGameProgress.SetFlag("LockedCircle");
                gGameProgress.ChangeScore("e_sidney_map_circle");
                SidneyUtil::UpdateLSRState();
            }
        }
    }
}

void SidneyAnalyze::AnalyzeMap_CheckAriesCompletion()
{
    // To complete Aries, the player must place a Rectangle with a certain position and size.
    for(size_t i = 0; i < mMap.zoomedOut.rectangles->GetCount(); ++i)
    {
        const UIRectangle& rectangle = mMap.zoomedOut.rectangles->Get(i);

        float centerDiffSq = (rectangle.center - kCircleCenter).GetLengthSq();
        float sizeDiff = Math::Abs(rectangle.size.x - kSquareAroundCircleSize);
        if(centerDiffSq < 20 * 20 && sizeDiff < 4)
        {
            // Clear mouse click action, forcing player to stop manipulating the rectangle.
            // The rectangle remains selected however.
            mMap.zoomedOutClickAction = MapState::ClickAction::None;

            // Set the rectangle to the correct position/size.
            // Note that the rectangle IS NOT locked yet, since the player can still rotate it.
            UIRectangle correctRectangle;
            correctRectangle.center = kCircleCenter;
            correctRectangle.size = Vector2::One * kSquareAroundCircleSize;
            correctRectangle.angle = rectangle.angle;

            mMap.zoomedOut.rectangles->Clear();
            mMap.zoomedOut.rectangles->Add(correctRectangle.center, correctRectangle.size, correctRectangle.angle);

            mMap.zoomedIn.rectangles->Clear();
            mMap.zoomedIn.rectangles->Add(mMap.ToZoomedInPoint(correctRectangle.center),
                                                   mMap.ToZoomedInPoint(correctRectangle.size),
                                                   correctRectangle.angle);

            // Grace is excited that we figured it out.
            gActionManager.ExecuteSheepAction("wait StartDialogue(\"02O7E2ZIS1\", 1)");

            // We completed Aries.
            gGameProgress.SetFlag("Aries");
            gGameProgress.SetFlag("SizedSquare");
            gGameProgress.ChangeScore("e_sidney_map_aries");
            SidneyUtil::UpdateLSRState();
        }
        //else
        //{
        //    printf("Center (%f, %f), Size (%f)\n", rectangle.center.x, rectangle.center.y, rectangle.size.x);
        //}
    }
}

void SidneyAnalyze::AnalyzeMap_CheckTaurusMeridianLine()
{
    // The game allows you to put down this line early in Aries, but it must be done to complete Taurus.
    // Find placed points that are close enough to the desired points.
    Vector2 serresPoint = mMap.zoomedIn.GetPlacedPointNearPoint(kTaurusSerresPoint);
    Vector2 meridianPoint = mMap.zoomedIn.GetPlacedPointNearPoint(kTaurusMeridianPoint);
    if(serresPoint != Vector2::Zero && meridianPoint != Vector2::Zero)
    {
        // Says "line is tangential to the circle."
        ShowAnalyzeMessage("MapLine2Note");

        // Remove points placed by the player.
        mMap.zoomedIn.points->Remove(serresPoint);
        mMap.zoomedIn.points->Remove(meridianPoint);
        mMap.zoomedOut.points->Remove(mMap.ToZoomedOutPoint(serresPoint));
        mMap.zoomedOut.points->Remove(mMap.ToZoomedOutPoint(meridianPoint));

        // It's possible for the player to place these points multiple times.
        // But only the first placement elicits exclamations from Grace.
        bool alreadyPlacedPoints = mMap.zoomedIn.GetPlacedPointNearPoint(kTaurusSerresPoint, true) != Vector2::Zero;
        if(!alreadyPlacedPoints)
        {
            // Grace says "Oh yeah, that's what the riddle means."
            gActionManager.ExecuteSheepAction("wait StartDialogue(\"02O3H2ZQB2\", 1)");

            // Add locked points.
            mMap.zoomedIn.lockedPoints->Add(kTaurusSerresPoint);
            mMap.zoomedIn.lockedPoints->Add(kTaurusMeridianPoint);
            mMap.zoomedOut.lockedPoints->Add(mMap.ToZoomedOutPoint(kTaurusSerresPoint));
            mMap.zoomedOut.lockedPoints->Add(mMap.ToZoomedOutPoint(kTaurusMeridianPoint));

            // Place a line segment between the points.
            mMap.zoomedIn.lines->Add(kTaurusSerresPoint, kTaurusMeridianPoint);
            mMap.zoomedOut.lines->Add(mMap.ToZoomedOutPoint(kTaurusSerresPoint),
                                          mMap.ToZoomedOutPoint(kTaurusMeridianPoint));

            // Update game state.
            gGameProgress.SetFlag("PlacedMeridianLine");
            gGameProgress.ChangeScore("e_sidney_map_serres");

            // NOTE: doing this doesn't complete Taurus - the player must rotate the square to align with these points.
        }
    }
}

void SidneyAnalyze::AnalyzeMap_CheckTaurusCompletion()
{
    bool placedMeridianLine = mMap.zoomedIn.GetPlacedPointNearPoint(kTaurusSerresPoint, true) != Vector2::Zero;
    if(placedMeridianLine)
    {
        const UIRectangle& rectangle = mMap.zoomedOut.rectangles->Get(0);

        // Convert the rectangle's angle to the range 0 to 2Pi.
        float angle = rectangle.angle;
        while(angle < 0.0f)
        {
            angle += Math::k2Pi;
        }
        angle = Math::Mod(angle, Math::k2Pi);
        //printf("Rectangle angle is %f\n", angle);

        // There are four possible orientations that are considered correct.
        if(Math::Approximately(angle, kSquareAroundCircleRotationRadians, 0.1f) ||
           Math::Approximately(angle, kSquareAroundCircleRotationRadians + Math::kPiOver2, 0.1f) ||
           Math::Approximately(angle, kSquareAroundCircleRotationRadians + Math::kPi, 0.1f) ||
           Math::Approximately(angle, kSquareAroundCircleRotationRadians + Math::kPi + Math::kPiOver2, 0.1f))
        {
            // Clear click action, forcing the player to stop rotating the rectangle.
            mMap.zoomedOutClickAction = MapState::ClickAction::None;

            // Also clear the selection - rectangle can no longer be selected.
            mMap.selectedRectangleIndex = -1;

            // "Lock in" the correct rectangle.
            mMap.zoomedOut.rectangles->Clear();
            mMap.zoomedIn.rectangles->Clear();

            mMap.zoomedOut.lockedRectangles->Add(rectangle.center, rectangle.size, kSquareAroundCircleRotationRadians);
            mMap.zoomedIn.lockedRectangles->Add(mMap.ToZoomedInPoint(rectangle.center),
                                                         mMap.ToZoomedInPoint(rectangle.size),
                                                         kSquareAroundCircleRotationRadians);

            // Grace is excited that we figured it out. And time moves forward a bit!
            gActionManager.ExecuteSheepAction("wait StartDialogue(\"02O7E2ZQB1\", 1)", [](const Action* action){
                gAudioManager.PlaySFX(gAssetManager.LoadAudio("CLOCKTIMEBLOCK.WAV"));
            });

            // Taurus is done.
            gGameProgress.SetFlag("Taurus");
            gGameProgress.SetFlag("LockedSquare");
            gGameProgress.ChangeScore("e_sidney_map_taurus");
            SidneyUtil::UpdateLSRState();
        }
    }
}

void SidneyAnalyze::AnalyzeMap_CheckGeminiAndCancerCompletion(float gridSize)
{
    // Bingo!
    if(gridSize == 8)
    {
        // Lock the placed grid.
        mMap.LockGrid();

        // Grace says "That's it! That's the chessboard."
        gActionManager.ExecuteSheepAction("wait StartDialogue(\"02OCL2ZJL1\", 1)", [this](const Action* action){

            // Gemini AND Cancer completed in one fell swoop.
            gGameProgress.SetFlag("Gemini");
            gGameProgress.SetFlag("Cancer");
            gGameProgress.SetFlag("PlacedGrid");
            gGameProgress.ChangeScore("e_sidney_map_gemini");
            SidneyUtil::UpdateLSRState();

            // NOTE: we want to hide Sidney for the timeblock change.
            // But doing it here causes an ugly single frame rendering the 3D scene before the timeblock video/ui appears.
            // This is now done after playing the opening movie of the following timeblock.
            //mSidney->SetActive(false);

            // Ok, this is the end of the 205P timeblock. All conditions are set to move on.
            // We should warp to the hallway now (where the next timeblock begins).
            // The timeblock complete script will move to the next timeblock.
            gLocationManager.ChangeLocation("HAL");
        });
    }
    else
    {
        // Grace says "I'm not sure about the size of the grid."
        gActionManager.ExecuteSheepAction("wait StartDialogue(\"02OD32ZGW1\", 1)");
    }
}

bool SidneyAnalyze::AnalyzeMap_CheckLeoCompletion()
{
    // Player must place two points near enough to these points and press "Analyze" to pass Leo.
    const Vector2 kLermitagePoint(676.0f, 696.0f);
    const Vector2 kPoussinTombPoint(936.0f, 1208.0f);

    // See if two placed points meet the criteria to finish Leo.
    Vector2 lermitagePoint = mMap.zoomedIn.GetPlacedPointNearPoint(kLermitagePoint);
    Vector2 poussinTombPoint = mMap.zoomedIn.GetPlacedPointNearPoint(kPoussinTombPoint);
    if(lermitagePoint != Vector2::Zero && poussinTombPoint != Vector2::Zero)
    {
        // Says "line passes through meridian at sunrise line."
        ShowAnalyzeMessage("MapLine3Note", Vector2(), HorizontalAlignment::Center);

        // Remove points placed by the player.
        mMap.zoomedIn.points->Remove(lermitagePoint);
        mMap.zoomedIn.points->Remove(poussinTombPoint);
        mMap.zoomedOut.points->Remove(mMap.ToZoomedOutPoint(lermitagePoint));
        mMap.zoomedOut.points->Remove(mMap.ToZoomedOutPoint(poussinTombPoint));

        // It's possible for the player to place these points multiple times.
        // But only the first placement elicits exclamations from Grace.
        bool alreadyPlacedPoints = mMap.zoomedIn.GetPlacedPointNearPoint(kLermitagePoint, true) != Vector2::Zero;
        if(!alreadyPlacedPoints)
        {
            // Grace says "Wow, it intersects the meridian at the same spot as the sunrise line!"
            gActionManager.ExecuteSheepAction("wait StartDialogue(\"02O3H2ZBY2\", 1)");

            // Add locked points.
            mMap.zoomedIn.lockedPoints->Add(kLermitagePoint);
            mMap.zoomedIn.lockedPoints->Add(kPoussinTombPoint);
            mMap.zoomedOut.lockedPoints->Add(mMap.ToZoomedOutPoint(kLermitagePoint));
            mMap.zoomedOut.lockedPoints->Add(mMap.ToZoomedOutPoint(kPoussinTombPoint));

            // Place a line segment between the points.
            mMap.zoomedIn.lines->Add(kLermitagePoint, kPoussinTombPoint);
            mMap.zoomedOut.lines->Add(mMap.ToZoomedOutPoint(kLermitagePoint),
                                          mMap.ToZoomedOutPoint(kPoussinTombPoint));

            // Leo is done!
            gGameProgress.ChangeScore("e_sidney_map_poussin");
            gGameProgress.SetFlag("Leo");
            gGameProgress.SetFlag("PlacedTombLine");
            SidneyUtil::UpdateLSRState();
            return true;
        }
    }
    return false;
}

void SidneyAnalyze::AnalyzeMap_CheckVirgoCompletion()
{
    // Player must place four points in the correct spots to pass Virgo.
    // See if placed points meet the criteria to finish Virgo.
    Vector2 point1 = mMap.zoomedIn.GetPlacedPointNearPoint(kTempleCorner1);
    Vector2 point2 = mMap.zoomedIn.GetPlacedPointNearPoint(kTempleCorner2);
    Vector2 point3 = mMap.zoomedIn.GetPlacedPointNearPoint(kTempleCorner3);
    Vector2 point4 = mMap.zoomedIn.GetPlacedPointNearPoint(kTempleCorner4);
    if(point1 != Vector2::Zero && point2 != Vector2::Zero && point3 != Vector2::Zero && point4 != Vector2::Zero)
    {
        // Says "points define 4-to-1 rectangle."
        ShowAnalyzeMessage("MapRectNote", Vector2(), HorizontalAlignment::Center);

        // Remove points placed by the player.
        mMap.zoomedIn.points->Remove(point1);
        mMap.zoomedIn.points->Remove(point2);
        mMap.zoomedIn.points->Remove(point3);
        mMap.zoomedIn.points->Remove(point4);
        mMap.zoomedOut.points->Remove(mMap.ToZoomedOutPoint(point1));
        mMap.zoomedOut.points->Remove(mMap.ToZoomedOutPoint(point2));
        mMap.zoomedOut.points->Remove(mMap.ToZoomedOutPoint(point3));
        mMap.zoomedOut.points->Remove(mMap.ToZoomedOutPoint(point4));

        // It's possible for the player to place these points multiple times.
        // But only the first placement elicits exclamations from Grace.
        bool alreadyPlacedPoints = mMap.zoomedIn.GetPlacedPointNearPoint(kTempleCorner1, true) != Vector2::Zero;
        if(!alreadyPlacedPoints)
        {
            // Grace says "That matches Wilkes seismic charts!"
            gActionManager.ExecuteSheepAction("wait StartDialogue(\"02O3H2ZKI2\", 1)", [](const Action* action){

                // This might end the timeblock.
                SidneyUtil::CheckForceExitSidney307A();
            });

            // Add locked points.
            mMap.zoomedIn.lockedPoints->Add(kTempleCorner1);
            mMap.zoomedIn.lockedPoints->Add(kTempleCorner2);
            mMap.zoomedIn.lockedPoints->Add(kTempleCorner3);
            mMap.zoomedIn.lockedPoints->Add(kTempleCorner4);
            mMap.zoomedOut.lockedPoints->Add(mMap.ToZoomedOutPoint(kTempleCorner1));
            mMap.zoomedOut.lockedPoints->Add(mMap.ToZoomedOutPoint(kTempleCorner2));
            mMap.zoomedOut.lockedPoints->Add(mMap.ToZoomedOutPoint(kTempleCorner3));
            mMap.zoomedOut.lockedPoints->Add(mMap.ToZoomedOutPoint(kTempleCorner4));

            // Place line segments between the points.
            mMap.zoomedIn.lines->Add(kTempleCorner1, kTempleCorner2);
            mMap.zoomedOut.lines->Add(mMap.ToZoomedOutPoint(kTempleCorner1), mMap.ToZoomedOutPoint(kTempleCorner2));

            mMap.zoomedIn.lines->Add(kTempleCorner2, kTempleCorner3);
            mMap.zoomedOut.lines->Add(mMap.ToZoomedOutPoint(kTempleCorner2), mMap.ToZoomedOutPoint(kTempleCorner3));

            mMap.zoomedIn.lines->Add(kTempleCorner3, kTempleCorner4);
            mMap.zoomedOut.lines->Add(mMap.ToZoomedOutPoint(kTempleCorner3), mMap.ToZoomedOutPoint(kTempleCorner4));

            mMap.zoomedIn.lines->Add(kTempleCorner4, kTempleCorner1);
            mMap.zoomedOut.lines->Add(mMap.ToZoomedOutPoint(kTempleCorner4), mMap.ToZoomedOutPoint(kTempleCorner1));

            // Virgo is done!
            gGameProgress.ChangeScore("e_sidney_map_virgo");
            gGameProgress.SetFlag("Virgo");
            gGameProgress.SetFlag("PlacedWalls");
            SidneyUtil::UpdateLSRState();
        }
    }
}

void SidneyAnalyze::AnalyzeMap_CheckLibraCompletion()
{
    // To complete Libra, the player must place a hexagram at a certain position, scale, and angle.
    for(size_t i = 0; i < mMap.zoomedOut.hexagrams->GetCount(); ++i)
    {
        const UIHexagram& hexagram = mMap.zoomedOut.hexagrams->Get(i);

        // Center/radius checks are the same as for the circle earlier...
        float centerDiffSq = (hexagram.center - kCircleCenter).GetLengthSq();
        float radiusDiff = Math::Abs(hexagram.radius - kCircleRadius);
        if(centerDiffSq < 20 * 20 && radiusDiff < 4)
        {
            // The hexagram needs to be in a specific rotation. But there are several valid rotations to get the correct visual effect.
            // In practice, it comes down to being at 30 degrees, or any multiple of 30 degrees before/after that.

            // Convert angle to degrees, within 0-360.
            float angle = Math::Abs(Math::Mod(hexagram.angle, Math::k2Pi));
            float degrees = Math::ToDegrees(angle);

            // Check all valid rotations. If any match, you got it!
            const float kCloseEnoughDegrees = 2.0f;
            if(Math::Approximately(degrees, kHexagramRotationDegrees, kCloseEnoughDegrees) ||
               Math::Approximately(degrees, kHexagramRotationDegrees + 60.0f, kCloseEnoughDegrees) ||
               Math::Approximately(degrees, kHexagramRotationDegrees + 120.0f, kCloseEnoughDegrees) ||
               Math::Approximately(degrees, kHexagramRotationDegrees + 180.0f, kCloseEnoughDegrees) ||
               Math::Approximately(degrees, kHexagramRotationDegrees + 240.0f, kCloseEnoughDegrees) ||
               Math::Approximately(degrees, kHexagramRotationDegrees + 300.0f, kCloseEnoughDegrees))
            {
                // Clear click action, forcing the player to stop rotating the hexagram.
                mMap.zoomedOutClickAction = MapState::ClickAction::None;

                // Also clear the selection - hexagram can no longer be selected.
                mMap.selectedHexagramIndex = -1;

                // "Lock in" the correct hexagram.
                mMap.zoomedOut.hexagrams->Clear();
                mMap.zoomedIn.hexagrams->Clear();

                mMap.zoomedOut.lockedHexagrams->Add(kCircleCenter, kCircleRadius, Math::ToRadians(kHexagramRotationDegrees));
                mMap.zoomedIn.lockedHexagrams->Add(mMap.ToZoomedInPoint(kCircleCenter), mMap.ToZoomedInDistance(kCircleRadius), Math::ToRadians(kHexagramRotationDegrees));

                // Grace says you got it right!
                gActionManager.ExecuteDialogueAction("02O1K2ZC73", 2, [](const Action* action){

                    // This might end the timeblock.
                    SidneyUtil::CheckForceExitSidney307A();
                });

                // And you just finished Libra.
                gGameProgress.ChangeScore("e_sidney_map_libra");
                gGameProgress.SetFlag("Libra");
                gGameProgress.SetFlag("LockedHexagram");
                SidneyUtil::UpdateLSRState();
            }
            //printf("Center (%f, %f), Size (%f), Angle (%f)\n", hexagram.center.x, hexagram.center.y, hexagram.radius, degrees);
        }
    }
}

bool SidneyAnalyze::AnalyzeMap_CheckScorpioPlaceTempleDivisions()
{
    // This can only be successfully done if the player has seen the Temple of Solomon email.
    if(gGameProgress.GetFlag("OpenedTempleDiagram") && !gGameProgress.GetFlag("PlacedTempleDivisions"))
    {
        // Player must place four points in the correct spots to pass Scorpio.
        // See if placed points meet the criteria to finish Virgo.
        Vector2 point1 = mMap.zoomedIn.GetPlacedPointNearPoint(kTempleDivisionPoint1);
        Vector2 point2 = mMap.zoomedIn.GetPlacedPointNearPoint(kTempleDivisionPoint2);
        Vector2 point3 = mMap.zoomedIn.GetPlacedPointNearPoint(kTempleDivisionPoint3);
        Vector2 point4 = mMap.zoomedIn.GetPlacedPointNearPoint(kTempleDivisionPoint4);
        if(point1 != Vector2::Zero && point2 != Vector2::Zero && point3 != Vector2::Zero && point4 != Vector2::Zero)
        {
            // Remove points placed by the player.
            mMap.zoomedIn.points->Remove(point1);
            mMap.zoomedIn.points->Remove(point2);
            mMap.zoomedIn.points->Remove(point3);
            mMap.zoomedIn.points->Remove(point4);
            mMap.zoomedOut.points->Remove(mMap.ToZoomedOutPoint(point1));
            mMap.zoomedOut.points->Remove(mMap.ToZoomedOutPoint(point2));
            mMap.zoomedOut.points->Remove(mMap.ToZoomedOutPoint(point3));
            mMap.zoomedOut.points->Remove(mMap.ToZoomedOutPoint(point4));

            // Add locked points.
            mMap.zoomedIn.lockedPoints->Add(kTempleDivisionPoint1);
            mMap.zoomedIn.lockedPoints->Add(kTempleDivisionPoint2);
            mMap.zoomedIn.lockedPoints->Add(kTempleDivisionPoint3);
            mMap.zoomedIn.lockedPoints->Add(kTempleDivisionPoint4);
            mMap.zoomedOut.lockedPoints->Add(mMap.ToZoomedOutPoint(kTempleDivisionPoint1));
            mMap.zoomedOut.lockedPoints->Add(mMap.ToZoomedOutPoint(kTempleDivisionPoint2));
            mMap.zoomedOut.lockedPoints->Add(mMap.ToZoomedOutPoint(kTempleDivisionPoint3));
            mMap.zoomedOut.lockedPoints->Add(mMap.ToZoomedOutPoint(kTempleDivisionPoint4));

            // Place line segments between the points to create the temple layout.
            mMap.zoomedIn.lines->Add(kTempleDivisionPoint1, kTempleDivisionPoint2);
            mMap.zoomedOut.lines->Add(mMap.ToZoomedOutPoint(kTempleDivisionPoint1), mMap.ToZoomedOutPoint(kTempleDivisionPoint2));

            mMap.zoomedIn.lines->Add(kTempleDivisionPoint3, kTempleDivisionPoint4);
            mMap.zoomedOut.lines->Add(mMap.ToZoomedOutPoint(kTempleDivisionPoint3), mMap.ToZoomedOutPoint(kTempleDivisionPoint4));

            // Grace says something like "that matches the temple diagram!"
            gActionManager.ExecuteSheepAction("wait StartDialogue(\"02O3H2ZR82\", 1)");

            // This makes progress towards Scorpio, but it doesn't complete it.
            gGameProgress.ChangeScore("e_sidney_map_temple");
            gGameProgress.SetFlag("PlacedTempleDivisions");
            return true;
        }
    }
    return false;
}

void SidneyAnalyze::AnalyzeMap_CheckScorpioCompletion(const Vector2& point)
{
    Vector2 sitePoint = mMap.zoomedIn.GetPlacedPointNearPoint(kTheSitePoint);
    if(sitePoint != Vector2::Zero)
    {
        // Replace placed point with locked actual point.
        mMap.zoomedIn.points->Remove(point);
        mMap.zoomedOut.points->Remove(mMap.ToZoomedOutPoint(point));
        mMap.zoomedIn.lockedPoints->Add(kTheSitePoint);
        mMap.zoomedOut.lockedPoints->Add(mMap.ToZoomedOutPoint(kTheSitePoint));

        // Change score and apply flags.
        gGameProgress.ChangeScore("e_sidney_map_scorpio");
        gGameProgress.SetFlag("MarkedTheSite");
        gGameProgress.SetFlag("Scorpio");
        SidneyUtil::UpdateLSRState();

        // Grace says "that's it, that's the site, I'll write down the coords."
        gActionManager.ExecuteDialogueAction("02O8O2ZRA1", 2, [this](const Action* action){

            // This popup is displayed a bit off-center.
            RectTransform* popupRT = static_cast<RectTransform*>(mSetTextPopup->GetTransform());
            popupRT->SetAnchor(AnchorPreset::TopLeft);
            popupRT->SetAnchoredPosition(120.0f, -127.5f);

            // Show the popup.
            mSetTextPopup->Show(SidneyUtil::GetAnalyzeLocalizer().GetText("SiteTextTitle"),
                                SidneyUtil::GetAnalyzeLocalizer().GetText("SiteTextPrompt"),
                                SidneyUtil::GetAnalyzeLocalizer().GetText("SiteText"), [this](){
                // Refresh the map so "The Site" label appears.
                mMap.RefreshImages();
            });
        });
    }
}

bool SidneyAnalyze::AnalyzeMap_CheckSagitariusCompletion()
{
    // The "Red Serpent" is a winding trail near the top of the map. Your goal here is to put points on the trail to mark it on the map.
    // You can place a lot of points here, but the only two required ones are at the start and end of the trail.
    // If you DO place additional points, a few optional points will also be locked in.
    const Vector2 kRedSerpentTailPoint(875.0f, 1163.0f);
    const Vector2 kRedSerpentHeadPoint(735.0f, 1365.0f);
    const Vector2 kOptionalPoint1(875.0f, 1206.0f);
    const Vector2 kOptionalPoint2(837.0f, 1238.0f);
    const Vector2 kOptionalPoint3(801.0f, 1267.0f);
    const Vector2 kOptionalPoint4(765.0f, 1301.0f);

    // See if we did the required points.
    Vector2 point1 = mMap.zoomedIn.GetPlacedPointNearPoint(kRedSerpentTailPoint);
    Vector2 point2 = mMap.zoomedIn.GetPlacedPointNearPoint(kRedSerpentHeadPoint);
    if(point1 != Vector2::Zero && point2 != Vector2::Zero)
    {
        // If you haven't completed Ophiuchus, the game allows you to place these points, but just responds with "I don't think I'm ready for that shape yet."
        // And your map points get cleared out.
        if(!gGameProgress.GetFlag("Ophiuchus"))
        {
            mMap.zoomedIn.points->Clear();
            mMap.zoomedOut.points->Clear();
            gActionManager.ExecuteDialogueAction("02O0I27NG1", 1);
        }
        else
        {
            // Otherwise, wow, you solved Sagitarius.

            // Add required locked points.
            mMap.zoomedIn.lockedPoints->Add(kRedSerpentTailPoint);
            mMap.zoomedIn.lockedPoints->Add(kRedSerpentHeadPoint);
            mMap.zoomedOut.lockedPoints->Add(mMap.ToZoomedOutPoint(kRedSerpentTailPoint));
            mMap.zoomedOut.lockedPoints->Add(mMap.ToZoomedOutPoint(kRedSerpentHeadPoint));

            // If the player placed points near the optional points, those also get locked.
            Vector2 optPoint1 = mMap.zoomedIn.GetPlacedPointNearPoint(kOptionalPoint1);
            if(optPoint1 != Vector2::Zero)
            {
                mMap.zoomedIn.lockedPoints->Add(kOptionalPoint1);
                mMap.zoomedOut.lockedPoints->Add(mMap.ToZoomedOutPoint(kOptionalPoint1));
            }
            Vector2 optPoint2 = mMap.zoomedIn.GetPlacedPointNearPoint(kOptionalPoint2);
            if(optPoint2 != Vector2::Zero)
            {
                mMap.zoomedIn.lockedPoints->Add(kOptionalPoint2);
                mMap.zoomedOut.lockedPoints->Add(mMap.ToZoomedOutPoint(kOptionalPoint2));
            }
            Vector2 optPoint3 = mMap.zoomedIn.GetPlacedPointNearPoint(kOptionalPoint3);
            if(optPoint3 != Vector2::Zero)
            {
                mMap.zoomedIn.lockedPoints->Add(kOptionalPoint3);
                mMap.zoomedOut.lockedPoints->Add(mMap.ToZoomedOutPoint(kOptionalPoint3));
            }
            Vector2 optPoint4 = mMap.zoomedIn.GetPlacedPointNearPoint(kOptionalPoint4);
            if(optPoint4 != Vector2::Zero)
            {
                mMap.zoomedIn.lockedPoints->Add(kOptionalPoint4);
                mMap.zoomedOut.lockedPoints->Add(mMap.ToZoomedOutPoint(kOptionalPoint4));
            }

            // Remove points placed by the player after locking all needed points.
            mMap.zoomedIn.points->Clear();
            mMap.zoomedOut.points->Clear();

            // Play Grace dialogue.
            gActionManager.ExecuteDialogueAction("0273H2ZRS2", 1);

            // Display analyze message.
            ShowAnalyzeMessage("MapLine4Note", Vector2(), HorizontalAlignment::Center);

            // Update score and flags.
            gGameProgress.ChangeScore("e_sidney_map_saggittarius");
            gGameProgress.SetFlag("PlacedSerpent");
            gGameProgress.SetFlag("Sagittarius");
            SidneyUtil::UpdateLSRState();

            // Refresh map so serpent image appears.
            mMap.RefreshImages();
            return true;
        }
    }
    return false;
}

void SidneyAnalyze::AnalyzeMap_FixOldSaveGames()
{
    // Older save files may have the circle in a slightly wrong spot.
    if(mMap.zoomedOut.lockedCircles->GetCount() > 0)
    {
        mMap.zoomedOut.lockedCircles->Clear();
        mMap.zoomedIn.lockedCircles->Clear();

        mMap.zoomedOut.lockedCircles->Add(kCircleCenter, kCircleRadius);
        mMap.zoomedIn.lockedCircles->Add(mMap.ToZoomedInPoint(kCircleCenter), mMap.ToZoomedInDistance(kCircleRadius));
    }

    // Older save files may have the square around the circle at a wrong angle.
    if(mMap.zoomedOut.lockedRectangles->GetCount() > 0)
    {
        mMap.zoomedOut.lockedRectangles->Clear();
        mMap.zoomedIn.lockedRectangles->Clear();

        mMap.zoomedOut.lockedRectangles->Add(kCircleCenter, Vector2::One * kSquareAroundCircleSize, kSquareAroundCircleRotationRadians);
        mMap.zoomedIn.lockedRectangles->Add(mMap.ToZoomedInPoint(kCircleCenter), mMap.ToZoomedInPoint(Vector2::One * kSquareAroundCircleSize), kSquareAroundCircleRotationRadians);
    }

    // Since the grid fills the square, it also needs to be updated due to the square angle changing.
    if(mMap.zoomedOut.lockedGrids->GetCount() > 0)
    {
        mMap.zoomedOut.lockedGrids->Clear();
        mMap.zoomedIn.lockedGrids->Clear();

        mMap.DrawGrid(8, true);
        mMap.LockGrid();
    }

    // The hexagram is placed relative to the square, so it's angle must also change.
    if(mMap.zoomedOut.lockedHexagrams->GetCount() > 0)
    {
        mMap.zoomedOut.lockedHexagrams->Clear();
        mMap.zoomedIn.lockedHexagrams->Clear();

        mMap.zoomedOut.lockedHexagrams->Add(kCircleCenter, kCircleRadius, Math::ToRadians(kHexagramRotationDegrees));
        mMap.zoomedIn.lockedHexagrams->Add(mMap.ToZoomedInPoint(kCircleCenter), mMap.ToZoomedInDistance(kCircleRadius), Math::ToRadians(kHexagramRotationDegrees));
    }

    // Because the square's angle changed, the temple corner points changed too.
    if(mMap.zoomedOut.lockedPoints->GetCount() >= 13)
    {
        mMap.zoomedIn.lockedPoints->Set(9, kTempleCorner1);
        mMap.zoomedIn.lockedPoints->Set(10, kTempleCorner2);
        mMap.zoomedIn.lockedPoints->Set(11, kTempleCorner3);
        mMap.zoomedIn.lockedPoints->Set(12, kTempleCorner4);

        mMap.zoomedOut.lockedPoints->Set(9, mMap.ToZoomedOutPoint(kTempleCorner1));
        mMap.zoomedOut.lockedPoints->Set(10, mMap.ToZoomedOutPoint(kTempleCorner2));
        mMap.zoomedOut.lockedPoints->Set(11, mMap.ToZoomedOutPoint(kTempleCorner3));
        mMap.zoomedOut.lockedPoints->Set(12, mMap.ToZoomedOutPoint(kTempleCorner4));
    }

    // And the lines between the temple corner points changed as well.
    if(mMap.zoomedOut.lines->GetCount() >= 7)
    {
        mMap.zoomedIn.lines->Set(3, kTempleCorner1, kTempleCorner2);
        mMap.zoomedIn.lines->Set(4, kTempleCorner2, kTempleCorner3);
        mMap.zoomedIn.lines->Set(5, kTempleCorner3, kTempleCorner4);
        mMap.zoomedIn.lines->Set(6, kTempleCorner4, kTempleCorner1);

        mMap.zoomedOut.lines->Set(3, mMap.ToZoomedOutPoint(kTempleCorner1), mMap.ToZoomedOutPoint(kTempleCorner2));
        mMap.zoomedOut.lines->Set(4, mMap.ToZoomedOutPoint(kTempleCorner2), mMap.ToZoomedOutPoint(kTempleCorner3));
        mMap.zoomedOut.lines->Set(5, mMap.ToZoomedOutPoint(kTempleCorner3), mMap.ToZoomedOutPoint(kTempleCorner4));
        mMap.zoomedOut.lines->Set(6, mMap.ToZoomedOutPoint(kTempleCorner4), mMap.ToZoomedOutPoint(kTempleCorner1));
    }

    // And the temple division points must change.
    if(mMap.zoomedOut.lockedPoints->GetCount() >= 17)
    {
        mMap.zoomedIn.lockedPoints->Set(13, kTempleDivisionPoint1);
        mMap.zoomedIn.lockedPoints->Set(14, kTempleDivisionPoint2);
        mMap.zoomedIn.lockedPoints->Set(15, kTempleDivisionPoint3);
        mMap.zoomedIn.lockedPoints->Set(16, kTempleDivisionPoint4);

        mMap.zoomedOut.lockedPoints->Set(13, mMap.ToZoomedOutPoint(kTempleDivisionPoint1));
        mMap.zoomedOut.lockedPoints->Set(14, mMap.ToZoomedOutPoint(kTempleDivisionPoint2));
        mMap.zoomedOut.lockedPoints->Set(15, mMap.ToZoomedOutPoint(kTempleDivisionPoint3));
        mMap.zoomedOut.lockedPoints->Set(16, mMap.ToZoomedOutPoint(kTempleDivisionPoint4));
    }

    // And the lines between the division points must change.
    if(mMap.zoomedOut.lines->GetCount() >= 9)
    {
        mMap.zoomedIn.lines->Set(7, kTempleDivisionPoint1, kTempleDivisionPoint2);
        mMap.zoomedIn.lines->Set(8, kTempleDivisionPoint3, kTempleDivisionPoint4);

        mMap.zoomedOut.lines->Set(7, mMap.ToZoomedOutPoint(kTempleDivisionPoint1), mMap.ToZoomedOutPoint(kTempleDivisionPoint2));
        mMap.zoomedOut.lines->Set(8, mMap.ToZoomedOutPoint(kTempleDivisionPoint3), mMap.ToZoomedOutPoint(kTempleDivisionPoint4));
    }

    // And finally, The Site point position is changed.
    if(mMap.zoomedOut.lockedPoints->GetCount() >= 18)
    {
        mMap.zoomedIn.lockedPoints->Set(17, kTheSitePoint);
        mMap.zoomedOut.lockedPoints->Set(17, mMap.ToZoomedOutPoint(kTheSitePoint));
    }
}
