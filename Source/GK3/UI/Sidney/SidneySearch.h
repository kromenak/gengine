//
// Clark Kromenaker
//
// UI for the search subscreen in Sidney.
//
#pragma once
#include <string>

#include "StringUtil.h"
#include "Vector2.h"

class Actor;
class Font;
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

    // Text input field.
    UITextInput* mTextInput = nullptr;

    // The panel that shows web page contents. Turned off when nothing to display.
    Actor* mWebPageRoot = nullptr;

    // A canvas within the web page root that contains web page widgets.
    // This is needed to mask things that go offscreen.
    UICanvas* mWebPageWidgetsCanvas = nullptr;

    // For scrolling web page contents that go offscreen.
    UIScrollRect* mWebScrollRect = nullptr;

    // All widgets created as part of generating a web page.
    // These need to be destroyed when no longer showing a page.
    std::vector<UIWidget*> mWebPageWidgets;

    // A map of search terms to web page asset names.
    std::string_map_ci<std::string> mSearchTerms;

    void ShowWebPage(const std::string& pageName);
    void ClearWebPage();

    UILabel* CreateWebPageText(const std::string& text, Font* font, const Vector2& pos, float width, std::string& link);

    void OnSearchButtonPressed();
    void OnResetButtonPressed();
};