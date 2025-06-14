//
// Clark Kromenaker
//
// UI for the search subscreen in Sidney.
//
#pragma once
#include <string>
#include <unordered_map>
#include <vector>

#include "SidneyMenuBar.h"
#include "StringUtil.h"
#include "Vector2.h"

class Actor;
class Font;
class PersistState;
class SidneyButton;
class SidneyPopup;
class UIButton;
class UICanvas;
class UILabel;
class UIScrollRect;
class UITextInput;
class UIWidget;

class SidneySearch
{
public:
    void Init(Actor* parent);

    void Show();
    void Hide();

    void OnUpdate(float deltaTime);

    void OnPersist(PersistState& ps);

private:
    // Root of this subscreen.
    Actor* mRoot = nullptr;

    // The menu bar.
    SidneyMenuBar mMenuBar;

    // Text input field for search.
    UITextInput* mTextInput = nullptr;

    // When you hover over a link, the target web page is displayed in the bottom right corner.
    UILabel* mLinkTargetLabel = nullptr;

    // The panel that shows web page contents. Turned off when nothing to display.
    Actor* mWebPageRoot = nullptr;

    // A canvas within the web page root that contains web page widgets.
    // This is needed to mask things that go offscreen.
    UICanvas* mWebPageWidgetsCanvas = nullptr;

    // For scrolling web page contents that go offscreen.
    UIScrollRect* mWebPageScrollRect = nullptr;

    // All widgets created as part of generating a web page.
    // These need to be destroyed when no longer showing a page.
    std::vector<UIWidget*> mWebPageWidgets;

    // A separate list of just the links on the current page, so we can detect hovering.
    std::unordered_map<UIButton*, std::string> mWebPageLinks;

    // A map of search terms to web page asset names.
    std::string_map_ci<std::string> mSearchTerms;

    // Back and forward buttons for navigating page history.
    SidneyButton* mHistoryBackButton = nullptr;
    SidneyButton* mHistoryForwardButton = nullptr;

    // A history of pages visited, with latest one on the back.
    static const int kMaxHistorySize = 10;
    std::vector<std::string> mHistory;

    // Keep track of our current position within the history when using the back/forward buttons.
    int mHistoryIndex = -1;

    // The name of the currently shown web page.
    std::string mCurrentPageName;

    // A popup to display alerts and other messages.
    SidneyPopup* mPopup = nullptr;

    // Visiting some pages may trigger dialogue if certain conditions are met.
    // This struct tracks those conditions and what to trigger.
    struct DialogueTrigger
    {
        // Flag requirements to trigger this dialogue. Can include "!" for flags NOT set.
        std::vector<std::string> requiredFlags;

        // Requirement to be within a certain LSR step.
        int lsrMin = -1;
        int lsrMax = -1;

        // The dialogue to play when this triggers.
        std::string licensePlate;

        // A flag to set when this triggers.
        std::string flagToSet;
    };
    std::string_map_ci<DialogueTrigger> mDialogueTriggers;

    // Visiting certain pages will grant score. There's no time-based or flag-based logic to this: you can visit the pages any time.
    // Unfortunately, this is not data-driven at all (it easily could have been by putting this data in SIDSEARCH.TXT). Hardcoding it here for now.
    std::string_map_ci<std::string> mWebPageScoreEvents = {
        { "Vampire.html", "e_sidney_search_vampires" },
        { "HolyGrail.html", "e_sidney_search_grail" },
        { "Quaternity.html", "e_sidney_search_quaternity" },
        { "StMichael.html", "e_sidney_search_st_michael" },
        { "Pythagoras.html", "e_sidney_search_pythagoras" },
        { "Asmodeus.html", "e_sidney_search_asmodeus" },
        { "Chessboard.html", "e_sidney_search_chessboard" },
        { "Duality.html", "e_sidney_search_duality" },
        { "Solomon.html", "e_sidney_search_solomon" },
        { "StVincent.html", "e_sidney_search_st_vincent" },
        { "TempleEntry.html", "e_sidney_search_temple_solomon" },
        { "TempleFloorPlan.html", "e_sidney_analysis_send_email_for_floor_plan" },
        { "Soul.html", "e_sidney_search_soul" },
        { "Hexagram.html", "e_sidney_search_hexagram" },
        { "Seal.html", "e_sidney_search_seal" },
    };

    // Visiting certain pages will set flags internally. There doesn't seem to be any additional logic besides visiting the page at any time.
    // Unfortunately, this is not data-driven at all. Hardcoding it here for now.
    std::string_map_ci<std::string> mWebPageFlagEvents = {
        { "Vampire.html", "Vampire" },
        { "RA.html", "RA" },
        { "Asmodeus.html", "Asmodeus" },
        { "Quaternity.html", "Quaternity" },
        { "StMichael.html", "StMichael" },
        { "Pythagoras.html", "Pythagoras" },
        { "Chessboard.html", "Chessboard" },
        { "Duality.html", "Duality" },
        { "Solomon.html", "Solomon" },
        { "StVincent.html", "StVincent" },
        { "TempleEntry.html", "SolomonTemple" },
        { "Soul.html", "Soul" },
        { "Hexagram.html", "Hexagram" },
        { "Seal.html", "Seal" },
        { "AlchemyTiltedSquare.html", "AlchemyTiltedSquare" },
        { "TempleFloorplan.html", "TempleFloorplan" }
    };

    void ShowWebPage(const std::string& pageName);
    void ClearWebPage();
    UILabel* CreateWebPageText(const std::string& text, Font* font, const Vector2& pos, float width, std::string& link);

    void TriggerWebPageEvents(const std::string& pageName);

    void AddToHistory(const std::string& pageName);
    void RefreshHistoryMenu();
    void RefreshHistoryButtons();

    void OnSearchButtonPressed();
    void OnResetButtonPressed();
    void OnBackButtonPressed();
    void OnForwardButtonPressed();
};