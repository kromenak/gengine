//
// Clark Kromenaker
//
// UI for the analyze subscreen of Sidney.
//
#pragma once
#include "SidneyMenuBar.h"

class Actor;
class SidneyButton;
class SidneyFiles;
struct SidneyFile;
class UIImage;
class UILabel;

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

    void AnalyzeFile();
};