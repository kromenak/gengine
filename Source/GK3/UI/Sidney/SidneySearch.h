//
// Clark Kromenaker
//
// UI for the search subscreen in Sidney.
//
#pragma once
#include <string>

#include "StringUtil.h"

class Actor;
class UIButton;
class UILabel;
class UITextInput;

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

    // A TEMP label to output search result.
    UILabel* mTempResultsLabel = nullptr;

    // A map of search terms to web pages.
    std::string_map_ci<std::string> mSearchTerms;

    void OnSearchButtonPressed(UIButton* button);
    void OnResetButtonPressed(UIButton* button);
};