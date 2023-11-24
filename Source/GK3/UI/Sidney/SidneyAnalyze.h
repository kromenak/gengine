//
// Clark Kromenaker
//
// UI for the analyze subscreen of Sidney.
//
#pragma once
#include "SidneyMenuBar.h"

#include "Vector2.h"

class Actor;
class SidneyButton;
class SidneyFiles;
struct SidneyFile;
class UIButton;
class UICircles;
class UIImage;
class UILabel;
class UILines;
class UIPoints;

class SidneyAnalyze
{
public:
    void Init(Actor* parent, SidneyFiles* sidneyFiles);

    void Show();
    void Hide();

    void OnUpdate(float deltaTime);

private:
    // Sidney Files module, so we can view and select files to analyze.
    SidneyFiles* mSidneyFiles = nullptr;

    // Root of this subscreen.
    Actor* mRoot = nullptr;

    // The menu bar.
    SidneyMenuBar mMenuBar;

    // A file that has been selected for analysis.
    SidneyFile* mAnalyzeFile = nullptr;

    // A button to press to perform analysis.
    SidneyButton* mAnalyzeButton = nullptr;

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
    Actor* mAnalyzeMessageWindow = nullptr;
    UILabel* mAnalyzeMessage = nullptr;
    
    void SetState(State state);
    void SetStateFromFile();

    void OnAnalyzeButtonPressed();

    void ShowAnalyzeMessage(const std::string& message);

    // ANALYZE IMAGE
    Actor* mAnalyzeImageWindow = nullptr;
    UIImage* mAnalyzeImage = nullptr;

    void AnalyzeImageInit();
    void AnalyzeImageEnter();
    void AnalyzeImage();

    // ANALYZE MAP
    Actor* mAnalyzeMapWindow = nullptr;

    // A status label that confirms certain map actions (e.g. placing points).
    UILabel* mMapStatusLabel = nullptr;
    float mMapStatusLabelTimer = 0.0f;

    bool mEnteringPoints = false;

    // The map screen is quite complex, so this helper class helps manage its state.
    struct MapState
    {
        struct UI
        {
            // The interactive area of the UI.
            UIButton* button = nullptr;

            // The background image of the map.
            UIImage* background = nullptr;

            // Points placed by the user (active and locked).
            UIPoints* points = nullptr;
            UIPoints* lockedPoints = nullptr;

            // Lines - only placed by system, not user.
            UILines* lines = nullptr;

            // Circles - placed and manipulated by the user.
            UICircles* circles = nullptr;
            UICircles* lockedCircles = nullptr;

            Vector2 GetLocalMousePos();
            Vector2 GetPlacedPointNearPoint(const Vector2& point);
        };
        UI zoomedOut;
        UI zoomedIn;

        int selectedCircleIndex = -1;

        enum class ClickAction
        {
            None,
            FocusMap,
            SelectShape,
            MoveShape,
            ResizeShape
        };
        ClickAction zoomedOutClickAction = ClickAction::None;

        Vector2 ZoomedOutToZoomedInPos(const Vector2& pos);
        Vector2 ZoomedInToZoomedOutPos(const Vector2& pos);

        std::string GetPointLatLongText(const Vector2& zoomedInPos);

        void AddShape(const std::string& shapeName);
        void EraseShape();
        bool IsShapeSelected();
    };
    MapState mMap;

    void AnalyzeMap_Init();
    void AnalyzeMap_EnterState();
    void AnalyzeMap_Update(float deltaTime);
    void AnalyzeMap_OnAnalyzeButtonPressed();

    void AnalyzeMap_SetStatusText(const std::string& text, float duration = 5.0f);
    void AnalyzeMap_SetPointStatusText(const std::string& baseMessage, const Vector2& zoomedInMapPos);
};