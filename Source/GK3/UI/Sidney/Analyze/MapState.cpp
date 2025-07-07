#include "MapState.h"

#include "ActionManager.h"
#include "AssetManager.h"
#include "CursorManager.h"
#include "GameProgress.h"
#include "InputManager.h"
#include "LineSegment.h"
#include "SidneyUtil.h"
#include "UICircles.h"
#include "UIGrids.h"
#include "UIHexagrams.h"
#include "UIImage.h"
#include "UIRectangles.h"

namespace
{
    // Map sizes, in pixels.
    const float kZoomedOutMapSize = 342.0f;
    const float kZoomedInMapSize = 1368.0f;

    // The color of selected shapes changes in the zoomed out view only.
    const Color32 kZoomedOutSelectedShapeColor(0, 0, 0, 255);

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

    bool IsPointOnEdgeOfHexagram(const UIHexagram& hexagram, const Vector2& point)
    {
        // Convert point to local space of the hexagram.
        // After doing this, we can just worry about solving as though the hexagram has no rotation.
        Vector3 localPoint = Matrix3::MakeRotateZ(-hexagram.angle).TransformPoint(point - hexagram.center);

        // Get six hexagram points.
        const float kAngleInterval = Math::k2Pi / 6;
        Vector3 points[6];
        for(int j = 0; j < 6; ++j)
        {
            float angle = j * kAngleInterval;
            points[j].x = hexagram.radius * Math::Sin(angle);
            points[j].y = hexagram.radius * Math::Cos(angle);
        }

        // Generate line segments for each one.
        LineSegment segments[6] = {
            LineSegment(points[0], points[2]),
            LineSegment(points[2], points[4]),
            LineSegment(points[4], points[0]),
            LineSegment(points[1], points[3]),
            LineSegment(points[3], points[5]),
            LineSegment(points[5], points[1])
        };
        for(int i = 0; i < 6; ++i)
        {
            Vector2 closestPoint = segments[i].GetClosestPoint(localPoint);
            float length = (closestPoint - localPoint).GetLengthSq();
            if(length < 16)
            {
                return true;
            }
        }
        return false;
    }
}

float MapState::ToZoomedInDistance(float distance)
{
    return (distance / kZoomedOutMapSize) * kZoomedInMapSize;
}

float MapState::ToZoomedOutDistance(float distance)
{
    return (distance / kZoomedInMapSize) * kZoomedOutMapSize;
}

Vector2 MapState::ToZoomedInPoint(const Vector2& pos)
{
    // Transform point from zoomed out map coordinate to zoomed in map coordinate.
    return (pos / kZoomedOutMapSize) * kZoomedInMapSize;
}

Vector2 MapState::ToZoomedOutPoint(const Vector2& pos)
{
    // Transform point from zoomed in map coordinate to zoomed out map coordinate.
    return (pos / kZoomedInMapSize) * kZoomedOutMapSize;
}

std::string MapState::GetPointLatLongText(const Vector2& zoomedInPos)
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

void MapState::AddShape(const std::string& shapeName)
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
            zoomedIn.circles->Add(kDefaultCirclePos, kDefaultCircleRadius);
            zoomedOut.circles->Add(ToZoomedOutPoint(kDefaultCirclePos), (kDefaultCircleRadius / kZoomedInMapSize) * kZoomedOutMapSize);
        }
    }
    else if(StringUtil::EqualsIgnoreCase(shapeName, "Rectangle"))
    {
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
            const Vector2 kDefaultRectanglePos(599.0f, 770.0f);
            const Vector2 kDefaultRectangleSize(400.0f, 400.0f);
            zoomedIn.rectangles->Add(kDefaultRectanglePos, kDefaultRectangleSize, 0.0f);
            zoomedOut.rectangles->Add(ToZoomedOutPoint(kDefaultRectanglePos), ToZoomedOutPoint(kDefaultRectangleSize), 0.0f);
        }
    }
    else if(StringUtil::EqualsIgnoreCase(shapeName, "Hexagram"))
    {
        if(!gGameProgress.GetFlag("Leo") || !gGameProgress.GetFlag("Virgo"))
        {
            // If you haven't finished Leo and Virgo, Grace will say she's not ready for this shape.
            gActionManager.ExecuteSheepAction("wait StartDialogue(\"02O0I27NG1\", 1)");
        }
        else if(gGameProgress.GetFlag("Libra"))
        {
            // If you try to place after Libra, Grace says another one isn't needed.
            gActionManager.ExecuteSheepAction("wait StartDialogue(\"02O0I27731\", 1)");
        }
        else
        {
            // You can only place a hexagram during Libra!
            const Vector2 kDefaultHexagramPos(599.0f, 770.0f);
            const float kDefaultHexagramRadius = 400.0f;
            zoomedIn.hexagrams->Add(kDefaultHexagramPos, kDefaultHexagramRadius, 0.0f);
            zoomedOut.hexagrams->Add(ToZoomedOutPoint(kDefaultHexagramPos), (kDefaultHexagramRadius / kZoomedInMapSize) * kZoomedOutMapSize, 0.0f);
        }
    }
}

void MapState::EraseSelectedShape()
{
    if(selectedCircleIndex >= 0)
    {
        selectedCircleIndex = -1;
        zoomedIn.circles->Clear();
        zoomedOut.circles->Clear();
    }
    if(selectedRectangleIndex >= 0)
    {
        selectedRectangleIndex = -1;
        zoomedIn.rectangles->Clear();
        zoomedOut.rectangles->Clear();
    }
    if(selectedHexagramIndex >= 0)
    {
        selectedHexagramIndex = -1;
        zoomedIn.hexagrams->Clear();
        zoomedOut.hexagrams->Clear();
    }
}

bool MapState::IsAnyShapeSelected()
{
    return selectedCircleIndex >= 0 || selectedRectangleIndex >= 0 || selectedHexagramIndex >= 0;
}

void MapState::ClearShapeSelection()
{
    if(selectedCircleIndex >= 0)
    {
        selectedCircleIndex = -1;
    }
    if(selectedRectangleIndex >= 0)
    {
        selectedRectangleIndex = -1;
    }
    if(selectedHexagramIndex >= 0)
    {
        selectedHexagramIndex = -1;
    }
    zoomedOut.ClearShapeSelection();
}

void MapState::DrawGrid(uint8_t size, bool fillShape)
{
    if(fillShape)
    {
        // The game acts like you can do this whenever you want, but it's actually only possible to fill a Rectangle in one specific scenario.
        if(zoomedOut.lockedRectangles->GetCount() > 0)
        {
            const UIRectangle& rect = zoomedOut.lockedRectangles->Get(0);
            zoomedOut.grids->Add(rect.center, rect.size, rect.angle, size, false);
            zoomedIn.grids->Add(ToZoomedInPoint(rect.center), ToZoomedInPoint(rect.size), rect.angle, size, false);
        }
    }
    else
    {
        // Not filling a shape, so just draw one grid that fills the entire map.
        Vector2 gridRectSize = Vector2::One * kZoomedInMapSize;
        Vector2 gridRectCenter = gridRectSize * 0.5f;
        zoomedIn.grids->Add(gridRectCenter, gridRectSize, 0.0f, size, true);

        gridRectSize = Vector2::One * kZoomedOutMapSize;
        gridRectCenter = gridRectSize * 0.5f;
        zoomedOut.grids->Add(gridRectCenter, gridRectSize, 0.0f, size, true);
    }
}

void MapState::LockGrid()
{
    if(zoomedOut.grids->GetCount() > 0)
    {
        const UIGrid& grid = zoomedOut.grids->Get(0);

        // Recreate grid in locked grids.
        zoomedOut.lockedGrids->Add(grid.center, grid.size, grid.angle, grid.divisions, grid.drawBorder);
        zoomedIn.lockedGrids->Add(ToZoomedInPoint(grid.center), ToZoomedInPoint(grid.size),
                                  grid.angle, grid.divisions, grid.drawBorder);

        // Clear non-locked grids.
        zoomedOut.grids->Clear();
        zoomedIn.grids->Clear();
    }
}

void MapState::ClearGrid()
{
    // Clear any placed grids.
    zoomedIn.grids->Clear();
    zoomedOut.grids->Clear();
}

void MapState::RefreshImages()
{
    bool siteVisible = gGameProgress.GetFlag("MarkedTheSite");
    zoomedIn.siteText[0]->SetEnabled(siteVisible);
    zoomedIn.siteText[1]->SetEnabled(siteVisible);
    zoomedOut.siteText[0]->SetEnabled(siteVisible);
    zoomedOut.siteText[1]->SetEnabled(siteVisible);

    bool serpentVisible = gGameProgress.GetFlag("PlacedSerpent");
    zoomedIn.serpentImage->SetEnabled(serpentVisible);
    zoomedOut.serpentImage->SetEnabled(serpentVisible);
}

void MapState::Init(Actor* parent)
{
    zoomedIn.Init(parent, true);
    zoomedOut.Init(parent, false);
}

void MapState::UpdateZoomedOutShapeManipulation()
{
    Vector2 zoomedOutMapPos = zoomedOut.GetLocalMousePos();

    // See if we clicked on a selectable shape. If so, it causes us to select it.
    if(gInputManager.IsMouseButtonLeadingEdge(InputManager::MouseButton::Left))
    {
        // Check for selecting a circle.
        for(size_t i = 0; i < zoomedOut.circles->GetCount(); ++i)
        {
            // The click pos must be along the edge of the circle, not in the center.
            const Circle& circle = zoomedOut.circles->Get(i);
            Vector2 edgePoint = circle.GetClosestSurfacePoint(zoomedOutMapPos);
            if((edgePoint - zoomedOutMapPos).GetLengthSq() < 2.5f * 2.5f)
            {
                if(selectedCircleIndex != i)
                {
                    ClearShapeSelection();
                    selectedCircleIndex = i;
                    zoomedOut.circles->SetColor(kZoomedOutSelectedShapeColor);
                    zoomedOutClickAction = MapState::ClickAction::SelectShape;
                    break;
                }
            }
        }

        // Check for selecting a rectangle.
        for(size_t i = 0; i < zoomedOut.rectangles->GetCount(); ++i)
        {
            const UIRectangle& rectangle = zoomedOut.rectangles->Get(i);
            if(IsPointOnEdgeOfRectangle(rectangle, zoomedOutMapPos))
            {
                if(selectedRectangleIndex != i)
                {
                    ClearShapeSelection();
                    selectedRectangleIndex = i;
                    zoomedOut.rectangles->SetColor(kZoomedOutSelectedShapeColor);
                    zoomedOutClickAction = MapState::ClickAction::SelectShape;
                    break;
                }
            }
        }

        // Check for selecting a hexagram.
        for(size_t i = 0; i < zoomedOut.hexagrams->GetCount(); ++i)
        {
            const UIHexagram& hexagram = zoomedOut.hexagrams->Get(i);
            if(IsPointOnEdgeOfHexagram(hexagram, zoomedOutMapPos))
            {
                if(selectedHexagramIndex != i)
                {
                    ClearShapeSelection();
                    selectedHexagramIndex = i;
                    zoomedOut.hexagrams->SetColor(kZoomedOutSelectedShapeColor);
                    zoomedOutClickAction = MapState::ClickAction::SelectShape;
                    break;
                }
            }
        }
    }

    // This isn't frequently used, but right-clicking actually de-selects the current shape.
    if(gInputManager.IsMouseButtonLeadingEdge(InputManager::MouseButton::Right))
    {
        ClearShapeSelection();
    }

    // We have a selected shape. See if we're hovering it and change the cursor if so.
    bool moveShapeCursor = false;
    bool resizeShapeCursor = false;
    bool rotateShapeCursor = false;
    if(IsAnyShapeSelected())
    {
        if(selectedCircleIndex >= 0)
        {
            const Circle& circle = zoomedOut.circles->Get(selectedCircleIndex);
            Vector2 edgePoint = circle.GetClosestSurfacePoint(zoomedOutMapPos);
            if((edgePoint - zoomedOutMapPos).GetLengthSq() < 2.5f * 2.5f || zoomedOutClickAction == MapState::ClickAction::ResizeShape)
            {
                resizeShapeCursor = true;

            }
            else if(circle.ContainsPoint(zoomedOutMapPos) || zoomedOutClickAction == MapState::ClickAction::MoveShape)
            {
                moveShapeCursor = true;
            }
        }
        else if(selectedRectangleIndex >= 0)
        {
            const UIRectangle& rectangle = zoomedOut.rectangles->Get(selectedRectangleIndex);
            if(IsPointOnEdgeOfRectangle(rectangle, zoomedOutMapPos))
            {
                resizeShapeCursor = true;

                // You are only allowed to resize the rectangle before Aries is complete.
                if(gGameProgress.GetFlag("Aries"))
                {
                    resizeShapeCursor = false;
                }
            }
            else if(IsPointInsideRectangle(rectangle, zoomedOutMapPos))
            {
                moveShapeCursor = true;

                // You are only allowed to move the rectangle before Aries is complete.
                if(gGameProgress.GetFlag("Aries"))
                {
                    moveShapeCursor = false;
                }
            }
            else
            {
                rotateShapeCursor = true;
            }
        }
        else if(selectedHexagramIndex >= 0)
        {
            const UIHexagram& hexagram = zoomedOut.hexagrams->Get(selectedHexagramIndex);
            if(IsPointOnEdgeOfHexagram(hexagram, zoomedOutMapPos))
            {
                resizeShapeCursor = true;
            }
            else if((zoomedOutMapPos - hexagram.center).GetLengthSq() < hexagram.radius * hexagram.radius)
            {
                moveShapeCursor = true;
            }
            else
            {
                rotateShapeCursor = true;
            }
        }
    }

    if(resizeShapeCursor || zoomedOutClickAction == MapState::ClickAction::ResizeShape)
    {
        gCursorManager.UseCustomCursor(gAssetManager.LoadCursor("C_DRAGRESIZED1.CUR"), 1);
    }
    else if(moveShapeCursor || zoomedOutClickAction == MapState::ClickAction::MoveShape)
    {
        gCursorManager.UseCustomCursor(gAssetManager.LoadCursor("C_DRAGMOVE.CUR"), 1);
    }
    else if(rotateShapeCursor || zoomedOutClickAction == MapState::ClickAction::RotateShape)
    {
        gCursorManager.UseCustomCursor(gAssetManager.LoadCursor("C_TURNRIGHT.CUR"), 1);
    }
    else
    {
        gCursorManager.UseDefaultCursor();
    }

    // Upon clicking the left mouse button, we commit to a certain action until the mouse button is later released.
    if(gInputManager.IsMouseButtonLeadingEdge(InputManager::MouseButton::Left) && zoomedOutClickAction == MapState::ClickAction::None)
    {
        if(moveShapeCursor)
        {
            zoomedOutClickAction = MapState::ClickAction::MoveShape;
        }
        else if(resizeShapeCursor)
        {
            zoomedOutClickAction = MapState::ClickAction::ResizeShape;
        }
        else if(rotateShapeCursor)
        {
            zoomedOutClickAction = MapState::ClickAction::RotateShape;
        }
        else
        {
            zoomedOutClickAction = MapState::ClickAction::FocusMap;
        }

        zoomedOutClickActionPos = zoomedOutMapPos;
        if(selectedCircleIndex >= 0)
        {
            zoomedOutClickShapeCenter = zoomedOut.circles->Get(selectedCircleIndex).center;
        }
        if(selectedRectangleIndex >= 0)
        {
            zoomedOutClickShapeCenter = zoomedOut.rectangles->Get(selectedRectangleIndex).center;
        }
        if(selectedHexagramIndex >= 0)
        {
            zoomedOutClickShapeCenter = zoomedOut.hexagrams->Get(selectedHexagramIndex).center;
        }
    }

    if(gInputManager.IsMouseButtonPressed(InputManager::MouseButton::Left))
    {
        if(zoomedOutClickAction == MapState::ClickAction::MoveShape)
        {
            if(selectedCircleIndex >= 0)
            {
                Circle zoomedOutCircle = zoomedOut.circles->Get(selectedCircleIndex);

                Vector3 expectedOffset = zoomedOutClickShapeCenter - zoomedOutClickActionPos;
                zoomedOutCircle.center = zoomedOutMapPos + expectedOffset;

                zoomedOut.circles->Clear();
                zoomedOut.circles->Add(zoomedOutCircle.center, zoomedOutCircle.radius);

                zoomedIn.circles->Clear();
                zoomedIn.circles->Add(ToZoomedInPoint(zoomedOutCircle.center), ToZoomedInDistance(zoomedOutCircle.radius));
            }
            if(selectedRectangleIndex >= 0)
            {
                UIRectangle zoomedOutRectangle = zoomedOut.rectangles->Get(selectedRectangleIndex);

                Vector3 expectedOffset = zoomedOutClickShapeCenter - zoomedOutClickActionPos;
                zoomedOutRectangle.center = zoomedOutMapPos + expectedOffset;

                zoomedOut.rectangles->Clear();
                zoomedOut.rectangles->Add(zoomedOutRectangle.center, zoomedOutRectangle.size, zoomedOutRectangle.angle);

                zoomedIn.rectangles->Clear();
                zoomedIn.rectangles->Add(ToZoomedInPoint(zoomedOutRectangle.center),
                                                       ToZoomedInPoint(zoomedOutRectangle.size),
                                                       zoomedOutRectangle.angle);
            }
            if(selectedHexagramIndex >= 0)
            {
                UIHexagram zoomedOutHexagram = zoomedOut.hexagrams->Get(selectedHexagramIndex);

                Vector3 expectedOffset = zoomedOutClickShapeCenter - zoomedOutClickActionPos;
                zoomedOutHexagram.center = zoomedOutMapPos + expectedOffset;

                zoomedOut.hexagrams->Clear();
                zoomedOut.hexagrams->Add(zoomedOutHexagram.center, zoomedOutHexagram.radius, zoomedOutHexagram.angle);

                zoomedIn.hexagrams->Clear();
                zoomedIn.hexagrams->Add(ToZoomedInPoint(zoomedOutHexagram.center), ToZoomedInDistance(zoomedOutHexagram.radius), zoomedOutHexagram.angle);
            }
        }
        else if(zoomedOutClickAction == MapState::ClickAction::ResizeShape)
        {
            if(selectedCircleIndex >= 0)
            {
                Circle zoomedOutCircle = zoomedOut.circles->Get(selectedCircleIndex);
                zoomedOutCircle.radius = (zoomedOutCircle.center - zoomedOutMapPos).GetLength();
                zoomedOut.circles->Clear();
                zoomedOut.circles->Add(zoomedOutCircle.center, zoomedOutCircle.radius);

                zoomedIn.circles->Clear();
                zoomedIn.circles->Add(ToZoomedInPoint(zoomedOutCircle.center), ToZoomedInDistance(zoomedOutCircle.radius));
            }
            if(selectedRectangleIndex >= 0)
            {
                UIRectangle zoomedOutRectangle = zoomedOut.rectangles->Get(selectedRectangleIndex);

                Vector2 mouseMoveOffset = zoomedOutMapPos - zoomedOutClickActionPos;
                Vector2 centerToPos = zoomedOutMapPos - zoomedOutRectangle.center;

                float distChange = mouseMoveOffset.GetLength();
                if(Vector2::Dot(mouseMoveOffset, centerToPos) < 0)
                {
                    distChange *= -1.0f;
                }
                zoomedOutClickActionPos = zoomedOutMapPos;

                zoomedOutRectangle.size.x += distChange;
                zoomedOutRectangle.size.y += distChange;

                zoomedOut.rectangles->Clear();
                zoomedOut.rectangles->Add(zoomedOutRectangle.center, zoomedOutRectangle.size, zoomedOutRectangle.angle);

                zoomedIn.rectangles->Clear();
                zoomedIn.rectangles->Add(ToZoomedInPoint(zoomedOutRectangle.center),
                                                       ToZoomedInPoint(zoomedOutRectangle.size),
                                                       zoomedOutRectangle.angle);
            }
            if(selectedHexagramIndex >= 0)
            {
                UIHexagram zoomedOutHexagram = zoomedOut.hexagrams->Get(selectedHexagramIndex);

                Vector2 mouseMoveOffset = zoomedOutMapPos - zoomedOutClickActionPos;
                Vector2 centerToPos = zoomedOutMapPos - zoomedOutHexagram.center;

                float distChange = mouseMoveOffset.GetLength();
                if(Vector2::Dot(mouseMoveOffset, centerToPos) < 0)
                {
                    distChange *= -1.0f;
                }
                zoomedOutClickActionPos = zoomedOutMapPos;

                zoomedOutHexagram.radius += distChange;

                zoomedOut.hexagrams->Clear();
                zoomedOut.hexagrams->Add(zoomedOutHexagram.center, zoomedOutHexagram.radius, zoomedOutHexagram.angle);

                zoomedIn.hexagrams->Clear();
                zoomedIn.hexagrams->Add(ToZoomedInPoint(zoomedOutHexagram.center), ToZoomedInDistance(zoomedOutHexagram.radius), zoomedOutHexagram.angle);
            }
        }
        else if(zoomedOutClickAction == MapState::ClickAction::RotateShape)
        {
            if(selectedRectangleIndex >= 0)
            {
                UIRectangle zoomedOutRectangle = zoomedOut.rectangles->Get(selectedRectangleIndex);

                Vector2 prevCenterToMouse = Vector2::Normalize(zoomedOutClickActionPos - zoomedOutClickShapeCenter);
                Vector2 centerToMouse = Vector2::Normalize(zoomedOutMapPos - zoomedOutClickShapeCenter);
                zoomedOutClickActionPos = zoomedOutMapPos;

                float angle = Math::Acos(Vector2::Dot(prevCenterToMouse, centerToMouse));
                Vector3 cross = Vector3::Cross(prevCenterToMouse, centerToMouse);
                if(cross.z < 0)
                {
                    angle *= -1.0f;
                }
                zoomedOutRectangle.angle += angle;

                zoomedOut.rectangles->Clear();
                zoomedOut.rectangles->Add(zoomedOutRectangle.center, zoomedOutRectangle.size, zoomedOutRectangle.angle);

                zoomedIn.rectangles->Clear();
                zoomedIn.rectangles->Add(ToZoomedInPoint(zoomedOutRectangle.center),
                                                       ToZoomedInPoint(zoomedOutRectangle.size),
                                                       zoomedOutRectangle.angle);
            }
            if(selectedHexagramIndex >= 0)
            {
                UIHexagram zoomedOutHexagram = zoomedOut.hexagrams->Get(selectedHexagramIndex);

                Vector2 prevCenterToMouse = Vector2::Normalize(zoomedOutClickActionPos - zoomedOutClickShapeCenter);
                Vector2 centerToMouse = Vector2::Normalize(zoomedOutMapPos - zoomedOutClickShapeCenter);
                zoomedOutClickActionPos = zoomedOutMapPos;

                float angle = Math::Acos(Vector2::Dot(prevCenterToMouse, centerToMouse));
                Vector3 cross = Vector3::Cross(prevCenterToMouse, centerToMouse);
                if(cross.z < 0)
                {
                    angle *= -1.0f;
                }
                zoomedOutHexagram.angle += angle;

                zoomedOut.hexagrams->Clear();
                zoomedOut.hexagrams->Add(zoomedOutHexagram.center, zoomedOutHexagram.radius, zoomedOutHexagram.angle);

                zoomedIn.hexagrams->Clear();
                zoomedIn.hexagrams->Add(ToZoomedInPoint(zoomedOutHexagram.center), ToZoomedInDistance(zoomedOutHexagram.radius), zoomedOutHexagram.angle);
            }
        }
        else if(zoomedOutClickAction == MapState::ClickAction::FocusMap)
        {
            Vector2 zoomedInMapPos = ToZoomedInPoint(zoomedOutMapPos);
            //printf("%f, %f\n", zoomedInPos.x, zoomedInPos.y);

            // The zoomed in view should center on the zoomed in pos.
            Vector2 halfViewSize(MapView::kZoomedInMapViewWidth * 0.5f, MapView::kZoomedInMapViewHeight * 0.5f);
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
            zoomedIn.mapImage->GetRectTransform()->SetAnchoredPosition(-zoomedInRect.GetMin());
        }
    }
}

void MapState::OnPersist(PersistState& ps)
{
    ps.Xfer(PERSIST_VAR(zoomedOut));
    ps.Xfer(PERSIST_VAR(zoomedIn));
}