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
class UIImage;
class UILabel;
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

    // A button to press to start analysis of a file.
    SidneyButton* mAnalyzeButton = nullptr;
    
    // Pre-analyze UI - displays the file name/image before you press the analysis button.
    Actor* mPreAnalyzeWindow = nullptr;
    UILabel* mPreAnalyzeTitleLabel = nullptr;
    UIImage* mPreAnalyzeItemImage = nullptr;

    // Map UI - used for the big riddle puzzle.
    Actor* mAnalyzeMapWindow = nullptr;

    // A status label that confirms certain map actions (e.g. placing points).
    UILabel* mMapStatusLabel = nullptr;
    float mMapStatusLabelTimer = 0.0f;

    bool mEnteringPoints = false;

    // When analyzing the map, it has a single state, but multiple views (zoomed in, zoomed out).
    struct MapState
    {
        struct UI
        {
            UIButton* button = nullptr;
            UIImage* background = nullptr;
            UIPoints* points = nullptr;

            Vector2 GetLocalPosFromMousePos();
        };
        UI zoomedOut;
        UI zoomedIn;

        Vector2 ZoomedOutToZoomedInPos(const Vector2& pos);
        Vector2 ZoomedInToZoomedOutPos(const Vector2& pos);
    };
    MapState mMap;

    void AnalyzeFile();
    void AnalyzeMap();

    void AnalyzeMapInit();
    void AnalyzeMapUpdate(float deltaTime);
    void AnalyzeMapSetStatusText(const std::string& text);
};