//
// Clark Kromenaker
//
// The map screen is quite complex, so this class tracks map state as it changes throughout the game.
// This class also handles the complexity of maintaining two synchronized map views (one zoomed in, one zoomed out).
//
#pragma once
#include <cstdint>
#include <string>

#include "MapView.h"
#include "Vector2.h"

class Actor;
class PersistState;

struct MapState
{
    // The zoomed out and in map views.
    MapView zoomedOut;
    MapView zoomedIn;

    // Index of selected shapes (in the UICircles/UIRectangles components).
    int selectedCircleIndex = -1;
    int selectedRectangleIndex = -1;
    int selectedHexagramIndex = -1;

    // Are we currently entering points?
    bool enteringPoints = false;

    // Tracks what action the current click is doing.
    enum class ClickAction
    {
        None,
        FocusMap,
        SelectShape,
        MoveShape,
        ResizeShape,
        RotateShape
    };
    ClickAction zoomedOutClickAction = ClickAction::None;

    // For move/resize/rotate shapes, it's helpful to store some extra state data between frames.
    Vector2 zoomedOutClickActionPos;
    Vector2 zoomedOutClickShapeCenter;

    // Points/Distances
    float ToZoomedInDistance(float distance);
    float ToZoomedOutDistance(float distance);
    Vector2 ToZoomedInPoint(const Vector2& pos);
    Vector2 ToZoomedOutPoint(const Vector2& pos);
    std::string GetPointLatLongText(const Vector2& zoomedInPos);

    // Shapes
    void AddShape(const std::string& shapeName);
    void EraseSelectedShape();
    bool IsAnyShapeSelected();
    void ClearShapeSelection();

    // Grids
    void DrawGrid(uint8_t size, bool fillShape);
    void LockGrid();
    void ClearGrid();

    // Images
    void RefreshImages();

    void Init(Actor* parent);
    void UpdateZoomedOutShapeManipulation();
    void OnPersist(PersistState& ps);
};