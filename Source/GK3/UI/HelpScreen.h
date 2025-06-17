//
// Clark Kromenaker
//
// This screen displays some help info about the controls for the game.
//
#pragma once
#include "Actor.h"

#include <string>
#include <vector>

#include "LayerManager.h"
#include "StringUtil.h"
#include "Vector2.h"

class Font;
class Texture;
class UIButton;
class UIImage;
class UILabel;
class UINineSlice;

class HelpScreen : public Actor
{
public:
    HelpScreen();

    void Show();
    void Hide();

private:
    Layer mLayer;

    // The box containing the help contents.
    UINineSlice* mBox = nullptr;

    // The title label for the current page.
    UILabel* mTitleLabel = nullptr;

    // Buttons for changing the current page.
    UIButton* mPrevButton = nullptr;
    UIButton* mNextButton = nullptr;

    // Button to exit the help screen.
    UIButton* mExitButton = nullptr;

    // This screen recycles three different types of labels/images to create each page.
    // Created instances of each type are stored in these lists.
    std::vector<UILabel*> mKeyLabels;
    std::vector<UILabel*> mUseLabels;
    std::vector<UIImage*> mImages;

    // Fonts used for key/use labels respectively.
    Font* mKeyFont = nullptr;
    Font* mUseFont = nullptr;

    // The help screen displays a series of pages, with each page divided into sections.
    // Essentially a data-driven way to display a bunch of text/images.
    struct Section
    {
        // Position on containing page.
        Vector2 position;

        // Width (controls text wrapping for key code and use text).
        float width = 0.0f;

        // An action identifier (e.g. "Inventory" that maps to a specific key on the keyboard).
        std::string actionId;

        // Instruction text explaining what an action does.
        std::string useText;

        // Alternatively, a texture might be displayed instead of text.
        Texture* texture = nullptr;
    };
    struct Page
    {
        // The page size.
        Vector2 size;

        // The page title.
        std::string title;

        // Positions for next/prev/exit buttons.
        Vector2 nextButtonPosition;
        Vector2 prevButtonPosition;
        Vector2 exitButtonPosition;

        // The sections that make up the page.
        std::string_map_ci<Section> sections;
    };

    // The pages, in order.
    std::vector<Page> mPages;

    // The page current being shown.
    int mPageIndex = 0;

    void SetPage(int pageIndex);
    void PreviousPage();
    void NextPage();

    std::string ActionIdToKey(const std::string& actionId);
};