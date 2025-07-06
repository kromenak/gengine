//
// Clark Kromenaker
//
// A tooltip is a piece of text that displays when you hover your cursor over something for awhile.
// It usually provides extra context/info about what some other UI element does.
//
#pragma once
#include "Actor.h"

#include "Vector4.h"

class Font;
class RectTransform;
class UILabel;
class UIWidget;

class Tooltip : public Actor
{
public:
    static Tooltip* Get();

    Tooltip();

    void Show(const std::string& text, UIWidget* hoverWidget = nullptr, bool debug = false);
    void Hide();

protected:
    void OnUpdate(float deltaTime) override;

private:
    // The root of the tooltip.
    RectTransform* mRoot = nullptr;

    // The label in the tooltip.
    UILabel* mLabel = nullptr;

    // Fonts used for the tooltop. Tooltips usually show in non-debug mode, but it's an option!
    Font* mFont = nullptr;
    Font* mDebugFont = nullptr;

    // Delays before the tooltip fully shows.
    // The "quick" variant applies if Show was called when this tooltip was already previously showing.
    float mShowDelay = 0.6f;
    float mQuickShowDelay = 0.2f;

    // The amount of padding to put around the text within the tooltip frame/border.
    Vector4 mTextPadding; // left, top, right, bottom

    // When "Show" is called, there is possibly a delay before we *actually* show.
    float mShowDelayTimer = 0.0f;

    // How long the tooltip has been hidden.
    float mHiddenTimer = 0.0f;

    // If set, the tooltip is associated with a specific widget that the mouse must be hovering.
    // If the mouse is no longer hovering this widget, the tooltip hides itself.
    UIWidget* mHoverWidget = nullptr;
};