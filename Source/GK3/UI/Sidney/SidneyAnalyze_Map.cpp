#include "SidneyAnalyze.h"

#include "Actor.h"
#include "ActionManager.h"
#include "AssetManager.h"
#include "AudioManager.h"
#include "Color32.h"
#include "CursorManager.h"
#include "GameProgress.h"
#include "RectTransform.h"
#include "SidneyFiles.h"
#include "UIButton.h"
#include "UICanvas.h"
#include "UICircles.h"
#include "UIImage.h"
#include "UILabel.h"
#include "UILines.h"
#include "UIPoints.h"
#include "UIRectangles.h"

namespace
{
    const float kZoomedOutMapSize = 342.0f;
    const float kZoomedInMapSize = 1368.0f;

    const float kZoomedInMapViewWidth = 271.0f;
    const float kZoomedInMapViewHeight = 324.0f;

    const float kHoverPointDist = 4.0f;
    const float kHoverPointDistSq = kHoverPointDist * kHoverPointDist;

    const Color32 kShapeZoomedOutColor(0, 255, 0, 255);
    const Color32 kShapeZoomedInColor(0, 255, 0, 190);

    const Color32 kSelectedShapeZoomedOutColor(0, 0, 0, 255);

    const Vector2 kPiscesCoustaussaPoint(405.0f, 1094.0f);
    const Vector2 kPiscesBezuPoint(301.0f, 385.0f);
    const Vector2 kPiscesBugarachPoint(991.0f, 327.0f);

    bool IsPointOnEdgeOfRectangle(const UIRectangle& rect, const Vector2& point)
    {
        // Convert point to local space of the rectangle.
        // After doing this, we can just worry about solving as though the rectangle has no rotation.
        Vector3 localPoint = Matrix3::MakeRotateZ(-rect.angle).TransformPoint(point - rect.center);

        // Calculate half width/height of the rect.
        float halfWidth = rect.size.x * 0.5f;
        float halfHeight = rect.size.y * 0.5f;

        // Get how far the point is from the center on both axes.
        float distFromCenterX = Math::Abs(localPoint.x);
        float distFromCenterY = Math::Abs(localPoint.y);

        // Determine whether the point was near any of the edges of the rectangle.
        const float kCloseToEdgeDist = 2.5f;
        bool nearLeftRightSides = Math::Abs(halfWidth - distFromCenterX) < kCloseToEdgeDist;
        bool nearTopBottomSides = Math::Abs(halfHeight - distFromCenterY) < kCloseToEdgeDist;

        // To actually be on the edge, one of those two bools must be true...
        // ...AND the distance on the opposite axis must be within the half-size of the shape.
        return (nearLeftRightSides && distFromCenterY <= halfHeight) || (nearTopBottomSides && distFromCenterX <= halfWidth);
    }

    bool IsPointInsideRectangle(const UIRectangle& rect, const Vector2& point)
    {
        // Convert point to local space of the rectangle.
        // After doing this, we can just worry about solving as though the rectangle has no rotation.
        Vector3 localPoint = Matrix3::MakeRotateZ(-rect.angle).TransformPoint(point - rect.center);

        // Calculate half width/height of the rect.
        float halfWidth = rect.size.x * 0.5f;
        float halfHeight = rect.size.y * 0.5f;

        // Get how far the point is from the center on both axes.
        float distFromCenterX = Math::Abs(localPoint.x);
        float distFromCenterY = Math::Abs(localPoint.y);

        // Inside the rectangle if within half sizes on both axes.
        return distFromCenterX < halfWidth && distFromCenterY < halfHeight;
    }
}

Vector2 SidneyAnalyze::MapState::UI::GetLocalMousePos()
{
    // Subtract min from mouse pos to get point relative to lower left corner.
    return gInputManager.GetMousePosition() - background->GetRectTransform()->GetWorldRect().GetMin();
}

Vector2 SidneyAnalyze::MapState::UI::GetPlacedPointNearPoint(const Vector2& point)
{
    // Iterate through points placed by the user.
    // Find one that is close enough to the desired point and return it.
    const float kCloseToPointDist = 20.0f;
    const float kCloseToPointDistSq = kCloseToPointDist * kCloseToPointDist;
    for(size_t i = 0; i < points->GetPointsCount(); ++i)
    {
        const Vector2& placedPoint = points->GetPoint(i);
        if((placedPoint - point).GetLengthSq() < kCloseToPointDistSq)
        {
            return placedPoint;
        }
    }

    // No point placed by user is near the passed in point - return Zero as a placeholder/default.
    return Vector2::Zero;
}

Vector2 SidneyAnalyze::MapState::ZoomedOutToZoomedInPos(const Vector2& pos)
{
    // Transform point from zoomed out map coordinate to zoomed in map coordinate.
    return (pos / kZoomedOutMapSize) * kZoomedInMapSize;
}

Vector2 SidneyAnalyze::MapState::ZoomedInToZoomedOutPos(const Vector2& pos)
{
    // Transform point from zoomed in map coordinate to zoomed out map coordinate.
    return (pos / kZoomedInMapSize) * kZoomedOutMapSize;
}

std::string SidneyAnalyze::MapState::GetPointLatLongText(const Vector2& zoomedInPos)
{
    // Get normalized position on map.
    Vector2 normPos = zoomedInPos / (kZoomedInMapSize - 1);

    // Convert to long/lat "points".
    // This is number of individual degrees between the min/max displayed on the map.
    const int kMaxLongitude = 521;
    const int kMaxLatitude = 281;
    const int kUnitsPerDegree = 60;
    int longValue = Math::RoundToInt(normPos.x * kMaxLongitude);
    int latValue = Math::RoundToInt(normPos.y * kMaxLatitude);

    // Convert to format of X'Y". Goes between 14'30" and 23'11".
    int longMajorValue = 0;
    int longMinorValue = 0;
    if(longValue < 30)
    {
        longMajorValue = 14;
        longMinorValue = longValue + 30;
    }
    else
    {
        longMajorValue = 15 + (longValue - 30) / kUnitsPerDegree;
        longMinorValue = (longValue - 30) % kUnitsPerDegree;
    }

    // Convert to format of X'Y". Goes between 52'33" and 57'14".
    int latMajorValue = 0;
    int latMinorValue = 0;
    if(latValue < 27)
    {
        latMajorValue = 52;
        latMinorValue = latValue + 33;
    }
    else
    {
        latMajorValue = 53 + (latValue - 27) / kUnitsPerDegree;
        latMinorValue = (latValue - 27) % kUnitsPerDegree;
    }

    // Format as a string and display as status text.
    std::string formatStr = SidneyUtil::GetAnalyzeLocalizer().GetText("MapLatLongText");
    return StringUtil::Format(formatStr.c_str(), 2, longMajorValue, longMinorValue, 42, latMajorValue, latMinorValue);
}

void SidneyAnalyze::MapState::AddShape(const std::string& shapeName)
{
    if(StringUtil::EqualsIgnoreCase(shapeName, "Triangle"))
    {
        // I don't think the game ever actually lets you place the triangle shape...
        // Grace says "I'm not ready for that shape yet."
        gActionManager.ExecuteSheepAction("wait StartDialogue(\"02O0I27NG1\", 1)");
    }
    else if(StringUtil::EqualsIgnoreCase(shapeName, "Circle"))
    {
        if(!gGameProgress.GetFlag("Aquarius"))
        {
            // If you try to place a Circle before Aquarius is done, Grace won't let you. 
            gActionManager.ExecuteSheepAction("wait StartDialogue(\"02O0I27NG1\", 1)");
        }
        else if(gGameProgress.GetFlag("Pisces"))
        {
            // If you try to place after Pisces, Grace says another one isn't needed.
            gActionManager.ExecuteSheepAction("wait StartDialogue(\"02O0I27731\", 1)");
        }
        else
        {
            // During Pisces is the only time you can place a circle!
            const Vector2 kDefaultCirclePos(599.0f, 770.0f);
            const float kDefaultCircleRadius = 200.0f;

            zoomedIn.circles->AddCircle(kDefaultCirclePos, kDefaultCircleRadius);
            zoomedOut.circles->AddCircle(ZoomedInToZoomedOutPos(kDefaultCirclePos), (kDefaultCircleRadius / kZoomedInMapSize) * kZoomedOutMapSize);
        }
    }
    else if(StringUtil::EqualsIgnoreCase(shapeName, "Rectangle"))
    {
        const Vector2 kDefaultRectanglePos(599.0f, 770.0f);
        const Vector2 kDefaultRectangleSize(400.0f, 400.0f);

        zoomedIn.rectangles->AddRectangle(kDefaultRectanglePos, kDefaultRectangleSize, Math::kPiOver4);
        zoomedOut.rectangles->AddRectangle(ZoomedInToZoomedOutPos(kDefaultRectanglePos), ZoomedInToZoomedOutPos(kDefaultRectangleSize), Math::kPiOver4);

        /*
        if(!gGameProgress.GetFlag("Pisces"))
        {
            // If you haven't finished Pisces, Grace will say she's not ready for this shape.
            gActionManager.ExecuteSheepAction("wait StartDialogue(\"02O0I27NG1\", 1)");
        }
        else if(gGameProgress.GetFlag("Aries"))
        {
            // If you try to place after Aries, Grace says another one isn't needed.
            gActionManager.ExecuteSheepAction("wait StartDialogue(\"02O0I27731\", 1)");
        }
        else
        {
            // You can only place a Rectangle during Aries!
        }
        */
    }
}

void SidneyAnalyze::MapState::EraseShape()
{
    if(selectedCircleIndex >= 0)
    {
        selectedCircleIndex = -1;

        zoomedIn.circles->ClearCircles();
        zoomedOut.circles->ClearCircles();
    }
    if(selectedRectangleIndex >= 0)
    {
        selectedRectangleIndex = -1;
        zoomedIn.rectangles->ClearRectangles();
        zoomedOut.rectangles->ClearRectangles();
    }
}

bool SidneyAnalyze::MapState::IsShapeSelected()
{
    return selectedCircleIndex >= 0 || selectedRectangleIndex >= 0;
}

void SidneyAnalyze::MapState::ClearSelectedShape()
{
    if(selectedCircleIndex >= 0)
    {
        selectedCircleIndex = -1;
        zoomedOut.circles->SetColor(kShapeZoomedOutColor);
    }
    if(selectedRectangleIndex >= 0)
    {
        selectedRectangleIndex = -1;
        zoomedOut.rectangles->SetColor(kShapeZoomedOutColor);
    }
}

void SidneyAnalyze::AnalyzeMap_Init()
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

        // Create locked rectangles renderer.
        {
            Actor* rectanglesActor = new Actor(TransformType::RectTransform);
            rectanglesActor->GetTransform()->SetParent(zoomedInMapBackground->GetTransform());

            mMap.zoomedIn.lockedRectangles = rectanglesActor->AddComponent<UIRectangles>();
            mMap.zoomedIn.lockedRectangles->SetColor(Color32(0, 0, 255, 192));

            mMap.zoomedIn.lockedRectangles->GetRectTransform()->SetAnchor(AnchorPreset::BottomLeft);
            mMap.zoomedIn.lockedRectangles->GetRectTransform()->SetAnchoredPosition(Vector2::Zero);
        }

        // Create rectangles renderer.
        {
            Actor* rectanglesActor = new Actor(TransformType::RectTransform);
            rectanglesActor->GetTransform()->SetParent(zoomedInMapBackground->GetTransform());

            mMap.zoomedIn.rectangles = rectanglesActor->AddComponent<UIRectangles>();
            mMap.zoomedIn.rectangles->SetColor(kShapeZoomedInColor);

            mMap.zoomedIn.rectangles->GetRectTransform()->SetAnchor(AnchorPreset::BottomLeft);
            mMap.zoomedIn.rectangles->GetRectTransform()->SetAnchoredPosition(Vector2::Zero);
        }

        // Create locked circles renderer.
        {
            Actor* circlesActor = new Actor(TransformType::RectTransform);
            circlesActor->GetTransform()->SetParent(zoomedInMapBackground->GetTransform());

            mMap.zoomedIn.lockedCircles = circlesActor->AddComponent<UICircles>();
            mMap.zoomedIn.lockedCircles->SetColor(Color32(0, 0, 255, 192));

            mMap.zoomedIn.lockedCircles->GetRectTransform()->SetAnchor(AnchorPreset::BottomLeft);
            mMap.zoomedIn.lockedCircles->GetRectTransform()->SetAnchoredPosition(Vector2::Zero);
        }

        // Create circles renderer.
        {
            Actor* circlesActor = new Actor(TransformType::RectTransform);
            circlesActor->GetTransform()->SetParent(zoomedInMapBackground->GetTransform());

            mMap.zoomedIn.circles = circlesActor->AddComponent<UICircles>();
            mMap.zoomedIn.circles->SetColor(kShapeZoomedInColor);

            mMap.zoomedIn.circles->GetRectTransform()->SetAnchor(AnchorPreset::BottomLeft);
            mMap.zoomedIn.circles->GetRectTransform()->SetAnchoredPosition(Vector2::Zero);
        }

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

        // Create locked rectangles renderer.
        {
            Actor* rectanglesActor = new Actor(TransformType::RectTransform);
            rectanglesActor->GetTransform()->SetParent(zoomedOutMapActor->GetTransform());

            mMap.zoomedOut.lockedRectangles = rectanglesActor->AddComponent<UIRectangles>();
            mMap.zoomedOut.lockedRectangles->SetColor(Color32(0, 0, 255, 255));

            mMap.zoomedOut.lockedRectangles->GetRectTransform()->SetAnchor(AnchorPreset::BottomLeft);
            mMap.zoomedOut.lockedRectangles->GetRectTransform()->SetAnchoredPosition(Vector2::Zero);
        }

        // Create rectangles renderer.
        {
            Actor* rectanglesActor = new Actor(TransformType::RectTransform);
            rectanglesActor->GetTransform()->SetParent(zoomedOutMapActor->GetTransform());

            mMap.zoomedOut.rectangles = rectanglesActor->AddComponent<UIRectangles>();
            mMap.zoomedOut.rectangles->SetColor(kShapeZoomedOutColor);

            mMap.zoomedOut.rectangles->GetRectTransform()->SetAnchor(AnchorPreset::BottomLeft);
            mMap.zoomedOut.rectangles->GetRectTransform()->SetAnchoredPosition(Vector2::Zero);
        }

        // Create locked circles renderer.
        {
            Actor* circlesActor = new Actor(TransformType::RectTransform);
            circlesActor->GetTransform()->SetParent(zoomedOutMapActor->GetTransform());

            mMap.zoomedOut.lockedCircles = circlesActor->AddComponent<UICircles>();
            mMap.zoomedOut.lockedCircles->SetColor(Color32(0, 0, 255, 255));

            mMap.zoomedOut.lockedCircles->GetRectTransform()->SetAnchor(AnchorPreset::BottomLeft);
            mMap.zoomedOut.lockedCircles->GetRectTransform()->SetAnchoredPosition(Vector2::Zero);
        }

        // Create circles renderer.
        {
            Actor* circlesActor = new Actor(TransformType::RectTransform);
            circlesActor->GetTransform()->SetParent(zoomedOutMapActor->GetTransform());

            mMap.zoomedOut.circles = circlesActor->AddComponent<UICircles>();
            mMap.zoomedOut.circles->SetColor(kShapeZoomedOutColor);

            mMap.zoomedOut.circles->GetRectTransform()->SetAnchor(AnchorPreset::BottomLeft);
            mMap.zoomedOut.circles->GetRectTransform()->SetAnchoredPosition(Vector2::Zero);
        }

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

void SidneyAnalyze::AnalyzeMap_EnterState()
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

void SidneyAnalyze::AnalyzeMap_Update(float deltaTime)
{
    if(!mAnalyzeMapWindow->IsActive()) { return; }

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
        if(!mMap.IsShapeSelected())
        {
            bool setText = false;
            for(size_t i = 0; i < mMap.zoomedOut.points->GetPointsCount(); ++i)
            {
                const Vector2& point = mMap.zoomedOut.points->GetPoint(i);
                float distToPointSq = (zoomedOutMapPos - point).GetLengthSq();
                if(distToPointSq < kHoverPointDistSq)
                {
                    AnalyzeMap_SetPointStatusText("MapHoverPointNote", mMap.ZoomedOutToZoomedInPos(point));
                    setText = true;
                    break;
                }
            }
            if(!setText)
            {
                for(size_t i = 0; i < mMap.zoomedOut.lockedPoints->GetPointsCount(); ++i)
                {
                    const Vector2& point = mMap.zoomedOut.lockedPoints->GetPoint(i);
                    float distToPointSq = (zoomedOutMapPos - point).GetLengthSq();
                    if(distToPointSq < kHoverPointDistSq)
                    {
                        AnalyzeMap_SetPointStatusText("MapHoverPointNote", mMap.ZoomedOutToZoomedInPos(point));
                        setText = true;
                        break;
                    }
                }
            }
        }

        // See if we clicked on a selectable shape. If so, it causes us to select it.
        if(gInputManager.IsMouseButtonLeadingEdge(InputManager::MouseButton::Left))
        {
            // Check for selecting a circle.
            for(size_t i = 0; i < mMap.zoomedOut.circles->GetCirclesCount(); ++i)
            {
                // The click pos must be along the edge of the circle, not in the center.
                const Circle& circle = mMap.zoomedOut.circles->GetCircle(i);
                Vector2 edgePoint = circle.GetClosestSurfacePoint(zoomedOutMapPos);
                if((edgePoint - zoomedOutMapPos).GetLengthSq() < 2.5f * 2.5f)
                {
                    if(mMap.selectedCircleIndex != i)
                    {
                        mMap.ClearSelectedShape();
                        mMap.selectedCircleIndex = i;
                        mMap.zoomedOut.circles->SetColor(kSelectedShapeZoomedOutColor);
                        mMap.zoomedOutClickAction = MapState::ClickAction::SelectShape;
                        break;
                    }
                }
            }

            // Check for selecting a rectangle.
            for(size_t i = 0; i < mMap.zoomedOut.rectangles->GetCount(); ++i)
            {
                const UIRectangle& rectangle = mMap.zoomedOut.rectangles->GetRectangle(i);
                if(IsPointOnEdgeOfRectangle(rectangle, zoomedOutMapPos))
                {
                    if(mMap.selectedRectangleIndex != i)
                    {
                        mMap.ClearSelectedShape();
                        mMap.selectedRectangleIndex = i;
                        mMap.zoomedOut.rectangles->SetColor(kSelectedShapeZoomedOutColor);
                        mMap.zoomedOutClickAction = MapState::ClickAction::SelectShape;
                        break;
                    }
                }
            }
        }

        // This isn't frequently used, but right-clicking actually de-selects the current shape.
        if(gInputManager.IsMouseButtonLeadingEdge(InputManager::MouseButton::Right))
        {
            mMap.ClearSelectedShape();
        }

        // We have a selected shape. See if we're hovering it and change the cursor if so.
        bool moveShapeCursor = false;
        bool resizeShapeCursor = false;
        bool rotateShapeCursor = false;
        if(mMap.IsShapeSelected())
        {
            if(mMap.selectedCircleIndex >= 0)
            {
                const Circle& circle = mMap.zoomedOut.circles->GetCircle(mMap.selectedCircleIndex);
                Vector2 edgePoint = circle.GetClosestSurfacePoint(zoomedOutMapPos);
                if((edgePoint - zoomedOutMapPos).GetLengthSq() < 2.5f * 2.5f || mMap.zoomedOutClickAction == MapState::ClickAction::ResizeShape)
                {
                    resizeShapeCursor = true;
                    
                }
                else if(circle.ContainsPoint(zoomedOutMapPos) || mMap.zoomedOutClickAction == MapState::ClickAction::MoveShape)
                {
                    moveShapeCursor = true;
                }
            }
            else if(mMap.selectedRectangleIndex >= 0)
            {
                const UIRectangle& rectangle = mMap.zoomedOut.rectangles->GetRectangle(mMap.selectedRectangleIndex);
                if(IsPointOnEdgeOfRectangle(rectangle, zoomedOutMapPos))
                {
                    resizeShapeCursor = true;
                }
                else if(IsPointInsideRectangle(rectangle, zoomedOutMapPos))
                {
                    moveShapeCursor = true;
                }
                else
                {
                    rotateShapeCursor = true;
                }
            }
        }

        if(resizeShapeCursor || mMap.zoomedOutClickAction == MapState::ClickAction::ResizeShape)
        {
            gCursorManager.UseCustomCursor(gAssetManager.LoadCursor("C_DRAGRESIZED1.CUR"));
        }
        else if(moveShapeCursor || mMap.zoomedOutClickAction == MapState::ClickAction::MoveShape)
        {
            gCursorManager.UseCustomCursor(gAssetManager.LoadCursor("C_DRAGMOVE.CUR"));
        }
        else if(rotateShapeCursor || mMap.zoomedOutClickAction == MapState::ClickAction::RotateShape)
        {
            gCursorManager.UseCustomCursor(gAssetManager.LoadCursor("C_TURNRIGHT.CUR"));
        }
        else
        {
            gCursorManager.UseDefaultCursor();
        }

        // Upon clicking the left mouse button, we commit to a certain action until the mouse button is later released.
        if(gInputManager.IsMouseButtonLeadingEdge(InputManager::MouseButton::Left) && mMap.zoomedOutClickAction == MapState::ClickAction::None)
        {
            if(moveShapeCursor)
            {
                mMap.zoomedOutClickAction = MapState::ClickAction::MoveShape;
            }
            else if(resizeShapeCursor)
            {
                mMap.zoomedOutClickAction = MapState::ClickAction::ResizeShape;
            }
            else if(rotateShapeCursor)
            {
                mMap.zoomedOutClickAction = MapState::ClickAction::RotateShape;
            }
            else
            {
                mMap.zoomedOutClickAction = MapState::ClickAction::FocusMap;
            }

            mMap.zoomedOutClickActionPos = zoomedOutMapPos;
            if(mMap.selectedCircleIndex >= 0)
            {
                mMap.zoomedOutClickShapeCenter = mMap.zoomedOut.circles->GetCircle(mMap.selectedCircleIndex).center;
            }
            if(mMap.selectedRectangleIndex >= 0)
            {
                mMap.zoomedOutClickShapeCenter = mMap.zoomedOut.rectangles->GetRectangle(mMap.selectedRectangleIndex).center;
            }
        }

        if(gInputManager.IsMouseButtonPressed(InputManager::MouseButton::Left))
        {
            if(mMap.zoomedOutClickAction == MapState::ClickAction::MoveShape)
            {
                if(mMap.selectedCircleIndex >= 0)
                {
                    Circle zoomedOutCircle = mMap.zoomedOut.circles->GetCircle(mMap.selectedCircleIndex);

                    Vector3 expectedOffset = mMap.zoomedOutClickShapeCenter - mMap.zoomedOutClickActionPos;
                    zoomedOutCircle.center = zoomedOutMapPos + expectedOffset;

                    mMap.zoomedOut.circles->ClearCircles();
                    mMap.zoomedOut.circles->AddCircle(zoomedOutCircle.center, zoomedOutCircle.radius);

                    mMap.zoomedIn.circles->ClearCircles();
                    mMap.zoomedIn.circles->AddCircle(mMap.ZoomedOutToZoomedInPos(zoomedOutCircle.center),
                                                     (zoomedOutCircle.radius / kZoomedOutMapSize) * kZoomedInMapSize);
                }
                if(mMap.selectedRectangleIndex >= 0)
                {
                    UIRectangle zoomedOutRectangle = mMap.zoomedOut.rectangles->GetRectangle(mMap.selectedRectangleIndex);

                    Vector3 expectedOffset = mMap.zoomedOutClickShapeCenter - mMap.zoomedOutClickActionPos;
                    zoomedOutRectangle.center = zoomedOutMapPos + expectedOffset;

                    mMap.zoomedOut.rectangles->ClearRectangles();
                    mMap.zoomedOut.rectangles->AddRectangle(zoomedOutRectangle.center, zoomedOutRectangle.size, zoomedOutRectangle.angle);

                    mMap.zoomedIn.rectangles->ClearRectangles();
                    mMap.zoomedIn.rectangles->AddRectangle(mMap.ZoomedOutToZoomedInPos(zoomedOutRectangle.center),
                                                           mMap.ZoomedOutToZoomedInPos(zoomedOutRectangle.size),
                                                           zoomedOutRectangle.angle);
                }
            }
            else if(mMap.zoomedOutClickAction == MapState::ClickAction::ResizeShape)
            {
                if(mMap.selectedCircleIndex >= 0)
                {
                    Circle zoomedOutCircle = mMap.zoomedOut.circles->GetCircle(mMap.selectedCircleIndex);
                    zoomedOutCircle.radius = (zoomedOutCircle.center - zoomedOutMapPos).GetLength();
                    mMap.zoomedOut.circles->ClearCircles();
                    mMap.zoomedOut.circles->AddCircle(zoomedOutCircle.center, zoomedOutCircle.radius);

                    mMap.zoomedIn.circles->ClearCircles();
                    mMap.zoomedIn.circles->AddCircle(mMap.ZoomedOutToZoomedInPos(zoomedOutCircle.center),
                                                     (zoomedOutCircle.radius / kZoomedOutMapSize) * kZoomedInMapSize);
                }
                if(mMap.selectedRectangleIndex >= 0)
                {
                    UIRectangle zoomedOutRectangle = mMap.zoomedOut.rectangles->GetRectangle(mMap.selectedRectangleIndex);

                    Vector2 mouseMoveOffset = zoomedOutMapPos - mMap.zoomedOutClickActionPos;
                    Vector2 centerToPos = zoomedOutMapPos - zoomedOutRectangle.center;

                    float distChange = mouseMoveOffset.GetLength();
                    if(Vector2::Dot(mouseMoveOffset, centerToPos) < 0)
                    {
                        distChange *= -1.0f;
                    }
                    mMap.zoomedOutClickActionPos = zoomedOutMapPos;

                    zoomedOutRectangle.size.x += distChange;
                    zoomedOutRectangle.size.y += distChange;

                    mMap.zoomedOut.rectangles->ClearRectangles();
                    mMap.zoomedOut.rectangles->AddRectangle(zoomedOutRectangle.center, zoomedOutRectangle.size, zoomedOutRectangle.angle);

                    mMap.zoomedIn.rectangles->ClearRectangles();
                    mMap.zoomedIn.rectangles->AddRectangle(mMap.ZoomedOutToZoomedInPos(zoomedOutRectangle.center),
                                                           mMap.ZoomedOutToZoomedInPos(zoomedOutRectangle.size),
                                                           zoomedOutRectangle.angle);
                }
            }
            else if(mMap.zoomedOutClickAction == MapState::ClickAction::RotateShape)
            {
                if(mMap.selectedRectangleIndex >= 0)
                {
                    UIRectangle zoomedOutRectangle = mMap.zoomedOut.rectangles->GetRectangle(mMap.selectedRectangleIndex);

                    Vector2 prevCenterToMouse = Vector2::Normalize(mMap.zoomedOutClickActionPos - mMap.zoomedOutClickShapeCenter);
                    Vector2 centerToMouse = Vector2::Normalize(zoomedOutMapPos - mMap.zoomedOutClickShapeCenter);
                    mMap.zoomedOutClickActionPos = zoomedOutMapPos;

                    float angle = Math::Acos(Vector2::Dot(prevCenterToMouse, centerToMouse));
                    Vector3 cross = Vector3::Cross(prevCenterToMouse, centerToMouse);
                    if(cross.z < 0)
                    {
                        angle *= -1.0f;
                    }
                    zoomedOutRectangle.angle += angle;

                    mMap.zoomedOut.rectangles->ClearRectangles();
                    mMap.zoomedOut.rectangles->AddRectangle(zoomedOutRectangle.center, zoomedOutRectangle.size, zoomedOutRectangle.angle);

                    mMap.zoomedIn.rectangles->ClearRectangles();
                    mMap.zoomedIn.rectangles->AddRectangle(mMap.ZoomedOutToZoomedInPos(zoomedOutRectangle.center),
                                                           mMap.ZoomedOutToZoomedInPos(zoomedOutRectangle.size),
                                                           zoomedOutRectangle.angle);
                }
            }
            else if(mMap.zoomedOutClickAction == MapState::ClickAction::FocusMap)
            {
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
        }
    }

    // Clicking on the zoomed in map may perform an action in some cases.
    if(mMap.zoomedIn.button->IsHovered() && gInputManager.IsMouseButtonLeadingEdge(InputManager::MouseButton::Left))
    {
        if(mEnteringPoints)
        {
            // Add point to zoomed in map at click pos.
            Vector2 zoomedInMapPos = mMap.zoomedIn.GetLocalMousePos();
            mMap.zoomedIn.points->AddPoint(zoomedInMapPos);
            printf("Add pt at %f, %f\n", zoomedInMapPos.x, zoomedInMapPos.y);

            // Also convert to zoomed out map position and add point there.
            mMap.zoomedOut.points->AddPoint(mMap.ZoomedInToZoomedOutPos(zoomedInMapPos));

            // When you place a point, the latitude/longitude of the point are displayed on-screen.
            AnalyzeMap_SetPointStatusText("MapEnterPointNote", zoomedInMapPos);
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

    // To complete Pisces, you must place three points on the map AND align a circle to them.
    bool aquariusDone = gGameProgress.GetFlag("Aquarius");
    bool piscesDone = gGameProgress.GetFlag("Pisces");
    if(aquariusDone && !piscesDone)
    {
        Vector2 coustaussaPoint = mMap.zoomedIn.GetPlacedPointNearPoint(kPiscesCoustaussaPoint);
        Vector2 bezuPoint = mMap.zoomedIn.GetPlacedPointNearPoint(kPiscesBezuPoint);
        Vector2 bugarachPoint = mMap.zoomedIn.GetPlacedPointNearPoint(kPiscesBugarachPoint);
        if(coustaussaPoint != Vector2::Zero && bezuPoint != Vector2::Zero && bugarachPoint != Vector2::Zero)
        {
            const Vector2 kCircleCenter(168.875f, 174.5f);
            const float kCircleRadius(121.0f);
            for(size_t i = 0; i < mMap.zoomedOut.circles->GetCirclesCount(); ++i)
            {
                const Circle& circle = mMap.zoomedOut.circles->GetCircle(i);

                float centerDiffSq = (circle.center - kCircleCenter).GetLengthSq();
                float radiusDiff = Math::Abs(circle.radius - kCircleRadius);
                if(centerDiffSq < 20 * 20 && radiusDiff < 4)
                {
                    // Put locked points on the zoomed in map.
                    mMap.zoomedIn.points->RemovePoint(coustaussaPoint);
                    mMap.zoomedIn.points->RemovePoint(bezuPoint);
                    mMap.zoomedIn.points->RemovePoint(bugarachPoint);

                    mMap.zoomedIn.lockedPoints->AddPoint(kPiscesCoustaussaPoint);
                    mMap.zoomedIn.lockedPoints->AddPoint(kPiscesBezuPoint);
                    mMap.zoomedIn.lockedPoints->AddPoint(kPiscesBugarachPoint);

                    // Same for the zoomed out map.
                    mMap.zoomedOut.points->RemovePoint(mMap.ZoomedInToZoomedOutPos(coustaussaPoint));
                    mMap.zoomedOut.points->RemovePoint(mMap.ZoomedInToZoomedOutPos(bezuPoint));
                    mMap.zoomedOut.points->RemovePoint(mMap.ZoomedInToZoomedOutPos(bugarachPoint));

                    mMap.zoomedOut.lockedPoints->AddPoint(mMap.ZoomedInToZoomedOutPos(kPiscesCoustaussaPoint));
                    mMap.zoomedOut.lockedPoints->AddPoint(mMap.ZoomedInToZoomedOutPos(kPiscesBezuPoint));
                    mMap.zoomedOut.lockedPoints->AddPoint(mMap.ZoomedInToZoomedOutPos(kPiscesBugarachPoint));

                    // Put locked circle on zoomed out map.
                    mMap.zoomedOut.circles->ClearCircles();
                    mMap.zoomedIn.circles->ClearCircles();
                    mMap.selectedCircleIndex = -1;

                    mMap.zoomedOut.lockedCircles->AddCircle(kCircleCenter, kCircleRadius);
                    mMap.zoomedIn.lockedCircles->AddCircle(mMap.ZoomedOutToZoomedInPos(kCircleCenter), (kCircleRadius / kZoomedOutMapSize) * kZoomedInMapSize);

                    // Grace is excited that we figured it out.
                    gActionManager.ExecuteSheepAction("wait StartDialogue(\"02OAG2ZJU2\", 2)", [](const Action* action) {
                        gAudioManager.PlaySFX(gAssetManager.LoadAudio("CLOCKTIMEBLOCK.WAV"));
                    });

                    // Show confirmation message.
                    std::string message = StringUtil::Format(SidneyUtil::GetAnalyzeLocalizer().GetText("MapCircleConfirmNote").c_str(),
                                                             mMap.GetPointLatLongText(mMap.ZoomedOutToZoomedInPos(kCircleCenter)).c_str());
                    ShowAnalyzeMessage(message);

                    // This also gets us the coordinates at the center of the circle as an inventory item.
                    gInventoryManager.AddInventoryItem("GRACE_COORDINATE_PAPER_1");

                    // We completed Pisces.
                    gGameProgress.SetFlag("Pisces");
                }
            }
        }
    }
}

void SidneyAnalyze::AnalyzeMap_OnAnalyzeButtonPressed()
{
    // The map analysis behavior depends on what part of the LSR riddle we're on.
    bool aquariusDone = gGameProgress.GetFlag("Aquarius");
    bool piscesDone = gGameProgress.GetFlag("Pisces");

    if(!aquariusDone) // Working on Aquarius
    {
        // Player must place two points near enough to these points and press "Analyze" to pass Aquarius.
        const Vector2 kRLCPoint(266.0f, 952.0f);
        const Vector2 kCDBPoint(653.0f, 1060.0f);
        const Vector2 kSunLineEndPoint(1336.0f, 1249.0f);

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

    // If you analyze the map, but there is not a more specific message to show, we always show this fallback at least.
    if(!mAnalyzeMessageWindow->IsActive())
    {
        // If points are placed, the message is different.
        if(mMap.zoomedIn.points->GetPointsCount() > 0 || mMap.zoomedIn.lockedPoints->GetPointsCount() > 0)
        {
            // Says "unclear how to analyze those points."
            ShowAnalyzeMessage("MapIndeterminateNote");
        }
        else
        {
            // Says "map is too complex to analyze - try adding points or shapes."
            ShowAnalyzeMessage("MapNoPrimitiveNote");
        }
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