//
// Clark Kromenaker
//
// Manages UI elements related to a single map view.
// There are two map views in the game - one for the zoomed in map, and one for the zoomed out map.
//
#pragma once
#include "Vector2.h"

class Actor;
class PersistState;
class UIButton;
class UIImage;
class UICircles;
class UIGrids;
class UIHexagrams;
class UILines;
class UIPoints;
class UIRectangles;

struct MapView
{
    // Zoomed in map view size, in pixels.
    static constexpr float kZoomedInMapViewWidth = 271.0f;
    static constexpr float kZoomedInMapViewHeight = 324.0f;

    // A button to detect when the player is interacting with this view.
    UIButton* button = nullptr;

    // The image of the map in the view.
    // Placed map elements are also children of this Actor!
    UIImage* mapImage = nullptr;

    // Points that have been placed on the map.
    // Locked points are a different color and not modifiable by players.
    UIPoints* points = nullptr;
    UIPoints* lockedPoints = nullptr;

    // Lines that have been placed on the map.
    // These are only placed by the system - players don't place these directly.
    UILines* lines = nullptr;

    // Circles that have been placed on the map.
    // Locked circles are no longer modifiable by players.
    UICircles* circles = nullptr;
    UICircles* lockedCircles = nullptr;

    // Rectangles that have been placed on the map.
    // Locked rectangles are no longer modifiable by players.
    UIRectangles* rectangles = nullptr;
    UIRectangles* lockedRectangles = nullptr;

    // Grids that have been placed on the map.
    // Locked grids are no longer modifiable by players.
    UIGrids* grids = nullptr;
    UIGrids* lockedGrids = nullptr;

    // Hexagrams that have been placed
    UIHexagrams* hexagrams = nullptr;
    UIHexagrams* lockedHexagrams = nullptr;

    // Images used to mark "the site" and the serpent on the map.
    UIImage* siteText[2] = { 0 };
    UIImage* serpentImage = nullptr;

    void Init(Actor* parent, bool zoomedIn);

    Vector2 GetLocalMousePos();
    Vector2 GetPlacedPointNearPoint(const Vector2& point, bool useLockedPoints = false);

    void ClearShapeSelection();

    void OnPersist(PersistState& ps);
};