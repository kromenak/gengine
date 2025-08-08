#include "MapView.h"

#include "AssetManager.h"
#include "InputManager.h"
#include "PersistState.h"
#include "UIButton.h"
#include "UICanvas.h"
#include "UICircles.h"
#include "UIGrids.h"
#include "UIHexagrams.h"
#include "UIImage.h"
#include "UILines.h"
#include "UIPoints.h"
#include "UIRectangles.h"
#include "UIUtil.h"

namespace
{
    // The distance from a placed point to a solution point that is considered "close enough" to trigger the solution.
    // For example, you must place 2 points to solve Aquarius, but the placed points don't need to be exact - just close enough.
    const float kCloseToPointDist = 20.0f;
    const float kCloseToPointDistSq = kCloseToPointDist * kCloseToPointDist;

    // The colors of unselected shapes in both views.
    const Color32 kZoomedOutShapeColor(0, 255, 0, 255);
    const Color32 kZoomedInShapeColor(0, 255, 0, 192);

    // The color of selected shapes changes in the zoomed out view only.
    const Color32 kZoomedOutSelectedShapeColor(0, 0, 0, 255);

    // The colors of locked grids in both views.
    const Color32 kZoomedInLockedGridColor(128, 128, 128, 192);
    const Color32 kZoomedOutLockedGridColor(128, 128, 128, 255);

    // The colors of locked shapes in both views.
    const Color32 kZoomedInLockedShapeColor(0, 0, 255, 192);
    const Color32 kZoomedOutLockedShapeColor(0, 0, 255, 255);

    // The colors of lines in both views.
    const Color32 kZoomedInLineColor(0, 0, 0, 192);
    const Color32 kZoomedOutLineColor(0, 0, 0, 255);
}

void MapView::Init(Actor* parent, bool zoomedIn)
{
    // Create a root Actor that will contain the map (whether zoomed in or not).
    // This always has a button on it, which we use to detect mouse hover/click on the map.
    button = UI::CreateWidgetActor<UIButton>(zoomedIn ? "Zoomed In Map" : "Zoomed Out Map", parent);

    // Whether zoomed or not, both maps need a masked canvas, though for different reasons.
    // Zoomed in: needs a masked canvas so that only a portion of the large map image shows at one time.
    // Zoomed out: needs a mask image so shapes added to map don't render outside map image.
    UICanvas* mapCanvas = button->GetOwner()->AddComponent<UICanvas>(1);
    mapCanvas->SetMasked(true);

    if(zoomedIn)
    {
        // Set the size/position of the root actor within the analyze map screen.
        mapCanvas->GetRectTransform()->SetAnchor(AnchorPreset::BottomLeft);
        mapCanvas->GetRectTransform()->SetAnchoredPosition(9.0f, 73.0f);
        mapCanvas->GetRectTransform()->SetSizeDelta(kZoomedInMapViewWidth, kZoomedInMapViewHeight);

        // Create big background image at full size.
        // Note that only a portion is visible due to the parent's canvas masking.
        mapImage = UI::CreateWidgetActor<UIImage>("MapImage", mapCanvas);
        mapImage->SetTexture(gAssetManager.LoadTexture("SIDNEYBIGMAP.BMP"), true);
        mapImage->GetRectTransform()->SetAnchor(AnchorPreset::BottomLeft);

        // Add "the site" text.
        // Do this before shapes/grids, since it should draw under those things.
        {
            siteText[0] = UI::CreateWidgetActor<UIImage>("SiteText1", mapImage);
            siteText[0]->SetTexture(gAssetManager.LoadTexture("MAPLG_THE.BMP"), true);
            siteText[0]->SetColor(Color32(255, 255, 255, 128));
            siteText[0]->GetRectTransform()->SetAnchor(AnchorPreset::BottomLeft);
            siteText[0]->GetRectTransform()->SetAnchoredPosition(804.0f, 1045.0f);
            siteText[0]->SetEnabled(false);

            siteText[1] = UI::CreateWidgetActor<UIImage>("SiteText2", mapImage);
            siteText[1]->SetTexture(gAssetManager.LoadTexture("MAPLG_SITE.BMP"), true);
            siteText[1]->SetColor(Color32(255, 255, 255, 128));
            siteText[1]->GetRectTransform()->SetAnchor(AnchorPreset::BottomLeft);
            siteText[1]->GetRectTransform()->SetAnchoredPosition(777.0f, 989.0f);
            siteText[1]->SetEnabled(false);
        }

        // Add serpent images.
        // Again, before shapes/grids so they appear above it.
        {
            serpentImage = UI::CreateWidgetActor<UIImage>("Serpent", mapImage);
            serpentImage->SetTexture(gAssetManager.LoadTexture("SERPENT.BMP"), true);
            serpentImage->GetRectTransform()->SetAnchor(AnchorPreset::BottomLeft);
            serpentImage->GetRectTransform()->SetAnchoredPosition(724.0f, 1159.0f);
            serpentImage->SetEnabled(false);
        }
    }
    else
    {
        // Position correctly in the analyze map screen.
        mapCanvas->GetRectTransform()->SetAnchor(AnchorPreset::BottomLeft);
        mapCanvas->GetRectTransform()->SetAnchoredPosition(289.0f, 55.0f);

        // For the zoomed out map, the whole map image is visible at all times.
        // So rather than making the map a child of the canvas, it can just be a component on the same actor.
        mapImage = button->GetOwner()->AddComponent<UIImage>();
        mapImage->SetTexture(gAssetManager.LoadTexture("SIDNEYLITTLEMAP.BMP"), true);

        // Add "the site" text.
        // Do this before shapes/grids, since it should draw under those things.
        {
            siteText[0] = UI::CreateWidgetActor<UIImage>("SiteText1", mapImage);
            siteText[0]->SetTexture(gAssetManager.LoadTexture("MAPSM_THE.BMP"), true);
            siteText[0]->GetRectTransform()->SetAnchor(AnchorPreset::BottomLeft);
            siteText[0]->GetRectTransform()->SetAnchoredPosition(198.0f, 257.0f);
            siteText[0]->SetEnabled(false);

            siteText[1] = UI::CreateWidgetActor<UIImage>("SiteText2", mapImage);
            siteText[1]->SetTexture(gAssetManager.LoadTexture("MAPSM_SITE.BMP"), true);
            siteText[1]->GetRectTransform()->SetAnchor(AnchorPreset::BottomLeft);
            siteText[1]->GetRectTransform()->SetAnchoredPosition(191.0f, 242.0f);
            siteText[1]->SetEnabled(false);
        }

        // Add serpent images.
        // Again, before shapes/grids so they appear above it.
        {
            serpentImage = UI::CreateWidgetActor<UIImage>("Serpent", mapImage);
            serpentImage->SetTexture(gAssetManager.LoadTexture("SERPLITMAP.BMP"), true);
            serpentImage->GetRectTransform()->SetAnchor(AnchorPreset::BottomLeft);
            serpentImage->GetRectTransform()->SetAnchoredPosition(178.0f, 290.0f);
            serpentImage->SetEnabled(false);
        }
    }

    // Create various shape renderers.
    // The main difference between the zoomed in and zoomed out maps are the colors used to render in some cases.
    // Create locked hexagrams renderer.
    {
        lockedHexagrams = UI::CreateWidgetActor<UIHexagrams>("LockedHexagrams", mapImage);
        lockedHexagrams->SetColor(zoomedIn ? kZoomedInLockedShapeColor : kZoomedOutLockedShapeColor);
        lockedHexagrams->GetRectTransform()->SetAnchor(AnchorPreset::BottomLeft);
    }

    // Create hexagrams renderer.
    {
        hexagrams = UI::CreateWidgetActor<UIHexagrams>("Hexagrams", mapImage);
        hexagrams->SetColor(zoomedIn ? kZoomedInShapeColor : kZoomedOutShapeColor);
        hexagrams->GetRectTransform()->SetAnchor(AnchorPreset::BottomLeft);
    }

    // Create locked grids renderer.
    {
        lockedGrids = UI::CreateWidgetActor<UIGrids>("LockedGrids", mapImage);
        lockedGrids->SetColor(zoomedIn ? kZoomedInLockedGridColor : kZoomedOutLockedGridColor);
        lockedGrids->GetRectTransform()->SetAnchor(AnchorPreset::BottomLeft);
    }

    // Create grids renderer.
    {
        grids = UI::CreateWidgetActor<UIGrids>("Grids", mapImage);
        grids->SetColor(zoomedIn ? kZoomedInShapeColor : kZoomedOutShapeColor);
        grids->GetRectTransform()->SetAnchor(AnchorPreset::BottomLeft);
    }

    // Create locked rectangles renderer.
    {
        lockedRectangles = UI::CreateWidgetActor<UIRectangles>("LockedRectangles", mapImage);
        lockedRectangles->SetColor(zoomedIn ? kZoomedInLockedShapeColor : kZoomedOutLockedShapeColor);
        lockedRectangles->GetRectTransform()->SetAnchor(AnchorPreset::BottomLeft);
    }

    // Create rectangles renderer.
    {
        rectangles = UI::CreateWidgetActor<UIRectangles>("Rectangles", mapImage);
        rectangles->SetColor(zoomedIn ? kZoomedInShapeColor : kZoomedOutShapeColor);
        rectangles->GetRectTransform()->SetAnchor(AnchorPreset::BottomLeft);
    }

    // Create locked circles renderer.
    {
        lockedCircles = UI::CreateWidgetActor<UICircles>("LockedCircles", mapImage);
        lockedCircles->SetColor(zoomedIn ? kZoomedInLockedShapeColor : kZoomedOutLockedShapeColor);
        lockedCircles->GetRectTransform()->SetAnchor(AnchorPreset::BottomLeft);
    }

    // Create circles renderer.
    {
        circles = UI::CreateWidgetActor<UICircles>("Circles", mapImage);
        circles->SetColor(zoomedIn ? kZoomedInShapeColor : kZoomedOutShapeColor);
        circles->GetRectTransform()->SetAnchor(AnchorPreset::BottomLeft);
    }

    // Create lines renderer.
    {
        lines = UI::CreateWidgetActor<UILines>("Lines", mapImage);
        lines->SetColor(zoomedIn ? kZoomedInLineColor : kZoomedOutLineColor);
        lines->GetRectTransform()->SetAnchor(AnchorPreset::BottomLeft);
    }

    // Create locked points renderer.
    {
        lockedPoints = UI::CreateWidgetActor<UIPoints>("LockedPoints", mapImage);
        lockedPoints->SetColor(zoomedIn ? kZoomedInLockedShapeColor : kZoomedOutLockedShapeColor);
        lockedPoints->GetRectTransform()->SetAnchor(AnchorPreset::BottomLeft);
    }

    // Create points renderer.
    {
        points = UI::CreateWidgetActor<UIPoints>("Points", mapImage);
        points->SetColor(zoomedIn ? kZoomedInShapeColor : kZoomedOutShapeColor);
        points->GetRectTransform()->SetAnchor(AnchorPreset::BottomLeft);
    }
}

Vector2 MapView::GetLocalMousePos()
{
    // The mouse position is relative to the bottom-left corner of the screen. And it's in "world space."
    // First, convert mouse pos to the map's local space. This is necessary due to potential parent RectTransform scaling.
    Vector2 localMousePos = mapImage->GetRectTransform()->GetWorldToLocalMatrix().TransformPoint(gInputManager.GetMousePosition());

    // Subtract min from mouse pos to get point relative to lower left corner of this rect.
    return localMousePos - mapImage->GetRectTransform()->GetRect().GetMin();
}

Vector2 MapView::GetPlacedPointNearPoint(const Vector2& point, bool useLockedPoints)
{
    // Iterate through points placed by the user.
    // Find one that is close enough to the desired point and return it.
    UIPoints* pts = useLockedPoints ? lockedPoints : points;
    for(size_t i = 0; i < pts->GetCount(); ++i)
    {
        const Vector2& placedPoint = pts->Get(i);
        if((placedPoint - point).GetLengthSq() < kCloseToPointDistSq)
        {
            return placedPoint;
        }
    }

    // No point placed by user is near the passed in point - return Zero as a placeholder/default.
    return Vector2::Zero;
}

void MapView::ClearShapeSelection()
{
    circles->SetColor(kZoomedOutShapeColor);
    rectangles->SetColor(kZoomedOutShapeColor);
    hexagrams->SetColor(kZoomedOutShapeColor);
}

void MapView::OnPersist(PersistState& ps)
{
    // We *could* add OnPersist methods to the various UI classes, so we can save points/lines/etc.
    // However, I'm unsure if I want UI code to be closely coupled to the persistence code.
    // For now, I'll just manually save/load these UI classes here.

    // Points
    {
        std::vector<Vector2> xferPoints;
        if(ps.IsSaving())
        {
            for(int i = 0; i < points->GetCount(); ++i)
            {
                xferPoints.push_back(points->Get(i));
            }
        }
        ps.Xfer("points", xferPoints);
        if(ps.IsLoading())
        {
            points->Clear();
            for(Vector2& p : xferPoints)
            {
                points->Add(p);
            }
        }
    }

    // Locked Points
    {
        std::vector<Vector2> xferPoints;
        if(ps.IsSaving())
        {
            for(int i = 0; i < lockedPoints->GetCount(); ++i)
            {
                xferPoints.push_back(lockedPoints->Get(i));
            }
        }
        ps.Xfer("lockedPoints", xferPoints);
        if(ps.IsLoading())
        {
            lockedPoints->Clear();
            for(Vector2& p : xferPoints)
            {
                lockedPoints->Add(p);
            }
        }
    }

    // Lines
    {
        std::vector<LineSegment> xferLines;
        if(ps.IsSaving())
        {
            for(int i = 0; i < lines->GetCount(); ++i)
            {
                xferLines.push_back(lines->Get(i));
            }
        }
        ps.Xfer("lines", xferLines);
        if(ps.IsLoading())
        {
            lines->Clear();
            for(LineSegment& ls : xferLines)
            {
                lines->Add(ls.start, ls.end);
            }
        }
    }

    // Circles
    {
        std::vector<Circle> xfer;
        if(ps.IsSaving())
        {
            for(int i = 0; i < circles->GetCount(); ++i)
            {
                xfer.push_back(circles->Get(i));
            }
        }
        ps.Xfer("circles", xfer);
        if(ps.IsLoading())
        {
            circles->Clear();
            for(Circle& circle : xfer)
            {
                circles->Add(circle.center, circle.radius);
            }
        }
    }

    // Locked Circles
    {
        std::vector<Circle> xfer;
        if(ps.IsSaving())
        {
            for(int i = 0; i < lockedCircles->GetCount(); ++i)
            {
                xfer.push_back(lockedCircles->Get(i));
            }
        }
        ps.Xfer("lockedCircles", xfer);
        if(ps.IsLoading())
        {
            lockedCircles->Clear();
            for(Circle& circle : xfer)
            {
                lockedCircles->Add(circle.center, circle.radius);
            }
        }
    }

    // Rectangles
    {
        std::vector<UIRectangle> xfer;
        if(ps.IsSaving())
        {
            for(int i = 0; i < rectangles->GetCount(); ++i)
            {
                xfer.push_back(rectangles->Get(i));
            }
        }
        ps.Xfer("rectangles", xfer);
        if(ps.IsLoading())
        {
            rectangles->Clear();
            for(UIRectangle& rect : xfer)
            {
                rectangles->Add(rect.center, rect.size, rect.angle);
            }
        }
    }

    // Locked Rectangles
    {
        std::vector<UIRectangle> xfer;
        if(ps.IsSaving())
        {
            for(int i = 0; i < lockedRectangles->GetCount(); ++i)
            {
                xfer.push_back(lockedRectangles->Get(i));
            }
        }
        ps.Xfer("lockedRectangles", xfer);
        if(ps.IsLoading())
        {
            lockedRectangles->Clear();
            for(UIRectangle& rect : xfer)
            {
                lockedRectangles->Add(rect.center, rect.size, rect.angle);
            }
        }
    }

    // Grids
    {
        std::vector<UIGrid> xfer;
        if(ps.IsSaving())
        {
            for(int i = 0; i < grids->GetCount(); ++i)
            {
                xfer.push_back(grids->Get(i));
            }
        }
        ps.Xfer("grids", xfer);
        if(ps.IsLoading())
        {
            grids->Clear();
            for(UIGrid& grid : xfer)
            {
                grids->Add(grid.center, grid.size, grid.angle, grid.divisions, grid.drawBorder);
            }
        }
    }

    // Locked Grids
    {
        std::vector<UIGrid> xfer;
        if(ps.IsSaving())
        {
            for(int i = 0; i < lockedGrids->GetCount(); ++i)
            {
                xfer.push_back(lockedGrids->Get(i));
            }
        }
        ps.Xfer("lockedGrids", xfer);
        if(ps.IsLoading())
        {
            lockedGrids->Clear();
            for(UIGrid& grid : xfer)
            {
                lockedGrids->Add(grid.center, grid.size, grid.angle, grid.divisions, grid.drawBorder);
            }
        }
    }

    // Hexagrams weren't added until version 2 of the save file.
    if(ps.GetFormatVersionNumber() >= 2)
    {
        // Hexagrams
        {
            std::vector<UIHexagram> xfer;
            if(ps.IsSaving())
            {
                for(int i = 0; i < hexagrams->GetCount(); ++i)
                {
                    xfer.push_back(hexagrams->Get(i));
                }
            }
            ps.Xfer("hexagrams", xfer);
            if(ps.IsLoading())
            {
                hexagrams->Clear();
                for(UIHexagram& hexagram : xfer)
                {
                    hexagrams->Add(hexagram.center, hexagram.radius, hexagram.angle);
                }
            }
        }

        // Locked Hexagrams
        {
            std::vector<UIHexagram> xfer;
            if(ps.IsSaving())
            {
                for(int i = 0; i < lockedHexagrams->GetCount(); ++i)
                {
                    xfer.push_back(lockedHexagrams->Get(i));
                }
            }
            ps.Xfer("lockedHexagrams", xfer);
            if(ps.IsLoading())
            {
                lockedHexagrams->Clear();
                for(UIHexagram& hexagram : xfer)
                {
                    lockedHexagrams->Add(hexagram.center, hexagram.radius, hexagram.angle);
                }
            }
        }
    }
}