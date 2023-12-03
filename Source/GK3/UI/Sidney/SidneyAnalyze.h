//
// Clark Kromenaker
//
// UI for the analyze subscreen of Sidney.
//
#pragma once
#include "SidneyMenuBar.h"
#include "Vector2.h"

class Actor;
class Sidney;
class SidneyButton;
class SidneyFiles;
struct SidneyFile;
class UIButton;
class UICircles;
class UIGrids;
class UIImage;
class UILabel;
class UILines;
class UIPoints;
class UIRectangles;

class SidneyAnalyze
{
public:
    void Init(Sidney* sidney, SidneyFiles* sidneyFiles);

    void Show();
    void Hide();

    void OnUpdate(float deltaTime);

private:
    // References to Sidney modules.
    Sidney* mSidney = nullptr;
    SidneyFiles* mSidneyFiles = nullptr;

    // Root of this subscreen.
    Actor* mRoot = nullptr;

    // The analyze screen menu bar.
    SidneyMenuBar mMenuBar;
    const size_t kFileDropdownIdx = 0;
    const size_t kTextDropdownIdx = 1;
    const size_t kGraphicDropdownIdx = 2;
    const size_t kMapDropdownIdx = 3;

    const size_t kTextDropdown_ExtractAnomaliesIdx = 0;
    const size_t kTextDropdown_TranslateIdx = 1;
    const size_t kTextDropdown_AnagramParserIdx = 2;
    const size_t kTextDropdown_AnalyzeTextIdx = 3;

    const size_t kGraphicDropdown_ViewGeometryIdx = 0;
    const size_t kGraphicDropdown_RotateShapeIdx = 1;
    const size_t kGraphicDropdown_ZoomClarifyIdx = 2;
    const size_t kGraphicDropdown_UseShapeIdx = 3;
    const size_t kGraphicDropdown_EraseShapeIdx = 4;

    // A file that has been selected for analysis.
    SidneyFile* mAnalyzeFile = nullptr;

    // A button to press to perform analysis.
    SidneyButton* mAnalyzeButton = nullptr;

    // The current state of the Analyze screen.
    // The Analyze screen contains several "sub-screens" for analyzing different types of data.
    enum class State
    {
        Empty,
        PreAnalyze,
        Map,
        Image
    };
    State mState = State::Empty;
    
    // Pre-analyze UI - displays the file name/image before you press the analyze button.
    Actor* mPreAnalyzeWindow = nullptr;
    UILabel* mPreAnalyzeTitleLabel = nullptr;
    UIImage* mPreAnalyzeItemImage = nullptr;

    // Message UI - shows a message in response to trying to analyze stuff.
    Actor* mAnalyzeMessageWindowRoot = nullptr;
    Actor* mAnalyzeMessageWindow = nullptr;
    UILabel* mAnalyzeMessage = nullptr;
    
    void SetState(State state);
    void SetStateFromFile();

    void OnAnalyzeButtonPressed();

    void ShowAnalyzeMessage(const std::string& message);

    // ANALYZE IMAGE
    Actor* mAnalyzeImageWindow = nullptr;
    UIImage* mAnalyzeImage = nullptr;

    void AnalyzeImage_Init();
    void AnalyzeImage_EnterState();
    void AnalyzeImage_OnAnalyzeButtonPressed();

    // ANALYZE MAP
    Actor* mAnalyzeMapWindow = nullptr;

    // A status label that confirms certain map actions (e.g. placing points).
    UILabel* mMapStatusLabel = nullptr;
    float mMapStatusLabelTimer = 0.0f;

    // The map screen is quite complex, so this helper class helps manage its state.
    struct MapState
    {
        struct View
        {
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

            Vector2 GetLocalMousePos();
            Vector2 GetPlacedPointNearPoint(const Vector2& point, bool useLockedPoints = false);
        };
        View zoomedOut;
        View zoomedIn;

        // Index of selected shapes (in the UICircles/UIRectangles components).
        int selectedCircleIndex = -1;
        int selectedRectangleIndex = -1;

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

        // Points
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
    };
    MapState mMap;

    void AnalyzeMap_Init();
    void AnalyzeMap_EnterState();

    void AnalyzeMap_Update(float deltaTime);
    void AnalyzeMap_UpdateZoomedOutMap(float deltaTime);
    void AnalyzeMap_UpdateZoomedInMap(float deltaTime);

    void AnalyzeMap_OnAnalyzeButtonPressed();
    void AnalyzeMap_OnUseShapePressed();
    void AnalyzeMap_OnEraseShapePressed();
    void AnalyzeMap_OnEnterPointsPressed();
    void AnalyzeMap_OnClearPointsPressed();
    void AnalyzeMap_OnDrawGridPressed();
    void AnalyzeMap_OnEraseGridPressed();

    void AnalyzeMap_SetStatusText(const std::string& text, float duration = 5.0f);
    void AnalyzeMap_SetPointStatusText(const std::string& baseMessage, const Vector2& zoomedInMapPos);
};