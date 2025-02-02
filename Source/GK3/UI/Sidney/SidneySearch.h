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
class SidneyButton;
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
    
    void ShowWebPage(const std::string& pageName);
    void ClearWebPage();
    UILabel* CreateWebPageText(const std::string& text, Font* font, const Vector2& pos, float width, std::string& link);

    void AddToHistory(const std::string& pageName);
    void RefreshHistoryMenu();
    void RefreshHistoryButtons();

    void OnSearchButtonPressed();
    void OnResetButtonPressed();
    void OnBackButtonPressed();
    void OnForwardButtonPressed();
};