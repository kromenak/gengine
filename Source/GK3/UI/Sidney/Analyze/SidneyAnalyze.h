//
// Clark Kromenaker
//
// UI for the analyze subscreen of Sidney.
//
#pragma once
#include "MapState.h"
#include "SidneyMenuBar.h"
#include "TextLayout.h" // HorizontalAlignment
#include "Vector2.h"

class Actor;
class PersistState;
class Sidney;
class SidneyAnagramParser;
class SidneyButton;
class SidneyFakeInputPopup;
class SidneyFiles;
struct SidneyFile;
class SidneyPopup;
class SidneyTranslate;
class UIButton;
class UIImage;
class UILabel;
class UIScrollRect;

class SidneyAnalyze
{
public:
    void Init(Sidney* sidney, SidneyFiles* sidneyFiles, SidneyTranslate* sidneyTranslate);

    void Show();
    void Hide();

    void OnUpdate(float deltaTime);

    void OnPersist(PersistState& ps);

private:
    // References to Sidney modules.
    Sidney* mSidney = nullptr;
    SidneyFiles* mSidneyFiles = nullptr;
    SidneyTranslate* mSidneyTranslate = nullptr;

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
    int mAnalyzeFileId = -1;

    // A button to press to perform analysis.
    SidneyButton* mAnalyzeButton = nullptr;

    // The current state of the Analyze screen.
    // The Analyze screen contains several "sub-screens" for analyzing different types of data.
    enum class State
    {
        Empty,
        PreAnalyze,
        Map,
        Image,
        Text,
        Audio
    };
    State mState = State::Empty;

    // Pre-analyze UI - displays the file name/image before you press the analyze button.
    Actor* mPreAnalyzeWindow = nullptr;
    UILabel* mPreAnalyzeTitleLabel = nullptr;
    UIImage* mPreAnalyzeItemImage = nullptr;

    // Message UI - shows a message in response to trying to analyze stuff.
    SidneyPopup* mAnalyzePopup = nullptr;

    // There are times when we need to show two popups at once. Sigh.
    SidneyPopup* mSecondaryAnalyzePopup = nullptr;

    // There's at least one time when a character enters input into a popup.
    SidneyFakeInputPopup* mSetTextPopup = nullptr;

    void SetState(State state);
    void SetStateFromFile();

    void OnAnalyzeButtonPressed();

    void ShowPreAnalyzeUI();
    void ShowAnalyzeMessage(const std::string& message, const Vector2& position = Vector2(), HorizontalAlignment textAlignment = HorizontalAlignment::Left, bool noButtons = false);

    // ANALYZE IMAGE
    Actor* mAnalyzeImageWindow = nullptr;
    UIImage* mAnalyzeImage = nullptr;
    UIImage* mAnalyzeVideoImages[3] = { 0 };

    void AnalyzeImage_Init();
    void AnalyzeImage_EnterState();
    void AnalyzeImage_OnAnalyzeButtonPressed();

    void AnalyzeImage_OnExtractAnomoliesPressed();
    void AnalyzeImage_OnAnalyzeTextPressed();

    void AnalyzeImage_OnViewGeometryButtonPressed();
    void AnalyzeImage_OnRotateShapeButtonPressed();
    void AnalyzeImage_OnZoomClarifyButtonPressed();

    void AnalyzeImage_ResetVideoImage(UIImage* image);
    void AnalyzeImage_PlayVideo(const std::string& videoName, UIImage* image, const std::string& finalTextureName, const std::function<void()>& finishCallback);

    // ANALYZE TEXT
    Actor* mAnalyzeTextWindow = nullptr;
    UILabel* mAnalyzeTextFileNameLabel = nullptr;
    UILabel* mAnalyzeTextLabel = nullptr;

    // The anagram parser.
    SidneyAnagramParser* mAnagramParser = nullptr;

    void AnalyzeText_Init();
    void AnalyzeText_EnterState();
    void AnalyzeText_ExitState();

    void AnalyzeText_OnAnalyzeButtonPressed();
    void AnalyzeText_OnTranslateButtonPressed();
    void AnalyzeText_OnAnagramParserPressed();

    // ANALYZE MAP
    Actor* mAnalyzeMapWindow = nullptr;

    // A status label that confirms certain map actions (e.g. placing points).
    UILabel* mMapStatusLabel = nullptr;
    float mMapStatusLabelTimer = 0.0f;

    // The map screen is quite complex, so this helper class helps manage its state.
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

    void AnalyzeMap_CheckAquariusCompletion();
    void AnalyzeMap_CheckPiscesCompletion();
    void AnalyzeMap_CheckAriesCompletion();
    void AnalyzeMap_CheckTaurusMeridianLine();
    void AnalyzeMap_CheckTaurusCompletion();
    void AnalyzeMap_CheckGeminiAndCancerCompletion(float gridSize);
    bool AnalyzeMap_CheckLeoCompletion();
    void AnalyzeMap_CheckVirgoCompletion();
    void AnalyzeMap_CheckLibraCompletion();
    bool AnalyzeMap_CheckScorpioPlaceTempleWalls();
    void AnalyzeMap_CheckScorpioCompletion(const Vector2& point);
    bool AnalyzeMap_CheckSagitariusCompletion();
};