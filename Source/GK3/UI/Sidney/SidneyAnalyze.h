//
// Clark Kromenaker
//
// UI for the analyze subscreen of Sidney.
//
#pragma once
#include "AudioManager.h"
#include "SidneyMenuBar.h"
#include "Vector2.h"
#include "UILabel.h"

class Actor;
class PersistState;
class Sidney;
class SidneyButton;
class SidneyFakeInputPopup;
class SidneyFiles;
struct SidneyFile;
class SidneyPopup;
class SidneyTranslate;
class UIButton;
class UICircles;
class UIGrids;
class UIHexagrams;
class UIImage;
class UILines;
class UIPoints;
class UIRectangles;
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
        Text
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

    // Root of the anagram parser window.
    Actor* mAnagramParserWindow = nullptr;

    // A label showing the text currently being parsed by the parser.
    UILabel* mAnagramParsingTextLabel = nullptr;

    // One label per letter of the anagram text.
    static const int kMaxAnagramLetters = 17;
    UILabel* mAnagramLetterLabels[kMaxAnagramLetters] = { 0 };

    // A label that shows feedback messages from the parser system.
    UILabel* mAnagramMessagesLabel = nullptr;

    // A scroll rect that contains the discovered anagram words.
    UIScrollRect* mAnagramWordsScrollRect = nullptr;

    // A set of selectable labels/buttons to choose words from.
    static const int kMaxAnagramWords = 161;
    UILabel* mAnagramWordLabels[kMaxAnagramWords] = { 0 };

    // The anagram erase and exit buttons. Need to be enabled/disabled at certain times.
    SidneyButton* mAnagramEraseButton = nullptr;
    SidneyButton* mAnagramExitButton = nullptr;

    // The anagram letters, with spaces stripped out.
    std::string mAnagramLetters;

    // A sound that plays when scrambling anagram letters.
    PlayingSoundHandle mAnagramScrambleSoundHandle;

    // A prefix for the words to populate in the anagram word list, how many to populate, and how many have been populated.
    std::string mAnagramWordsPrefix;
    int mAnagramWordsCount = 0;
    int mAnagramWordsIndex = 0;

    // Timers for how frequently we scamble the letters and how frequently we add a new word to the list.
    const float kAnagramScrambleIntervalSeconds = 0.1f;
    const float kAddAnagramWordInitialDelay = 4.0f;
    const float kAddAnagramWordIntervalSeconds = 0.15f;
    float mAnagramScrambleTimer = 0.0f;
    float mAddAnagramWordTimer = 0.0f;

    // Labels to display each selected word.
    static const int kMaxSelectedWords = 10;
    UILabel* mAnagramSelectedWordLabels[kMaxSelectedWords] = { 0 };

    // The indexes (in the word labels array) of words that have been selected by the player.
    std::vector<int> mSelectedWordIndexes;

    // A label that displays the deciphered text as one string.
    UILabel* mAnagramDecipheredTextLabel = nullptr;

    void AnalyzeText_Init();
    void AnalyzeText_EnterState();
    void AnalyzeText_Update(float deltaTime);

    void AnalyzeText_OnAnalyzeButtonPressed();
    void AnalyzeText_OnTranslateButtonPressed();
    void AnalyzeText_OnAnagramParserPressed();

    void AnalyzeText_OnAnagramParserWordSelected(int index);
    void AnalyzeText_OnAnagramEraseButtonPressed();
    void AnalyzeTest_RefreshSelectedWordsAndAvailableWords();
    void AnalyzeText_RefreshAvailableWordPositions();

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

            // Hexagrams that have been placed 
            UIHexagrams* hexagrams = nullptr;
            UIHexagrams* lockedHexagrams = nullptr;

            UIImage* siteText[2] = { 0 };

            Vector2 GetLocalMousePos();
            Vector2 GetPlacedPointNearPoint(const Vector2& point, bool useLockedPoints = false);

            void OnPersist(PersistState& ps);
        };
        View zoomedOut;
        View zoomedIn;

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

        // Images
        void RefreshTheSiteLabels();

        void OnPersist(PersistState& ps);
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