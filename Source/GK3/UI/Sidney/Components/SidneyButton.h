//
// Clark Kromenaker
//
// Throughout Sidney, there is a recurring button that has some special behavior:
// 1) Uses a nine-slice gray/yellow border.
// 2) The border highlights when hovered.
// 3) Clicking the button plays a short animation before actually performing the button action.
// 4) The button can be clicked programmatically for some short cutscenes.
//
// This class encompasses that behavior in a reusable Actor.
//
#pragma once
#include "Actor.h"

#include <functional>

#include "UILabel.h"

class Audio;
class Font;
class RectTransform;
class UIButton;
class UINineSlice;

class SidneyButton : public Actor
{
public:
    SidneyButton(const std::string& name, Actor* parent);
    SidneyButton(Actor* parent);
    void PrepareToDestroy();

    RectTransform* GetRectTransform() const { return static_cast<RectTransform*>(GetTransform()); }
    float GetWidth() const { return GetRectTransform()->GetSizeDelta().x; }
    void SetWidth(float width) { GetRectTransform()->SetSizeDeltaX(width); }
    float GetHeight() const { return GetRectTransform()->GetSizeDelta().y; }
    void SetHeight(float height) { GetRectTransform()->SetSizeDeltaY(height); }

    UILabel* GetLabel() const { return mLabel; }
    void SetFont(Font* font, Font* disabledFont = nullptr);
    void SetText(const std::string& text);
    void SetTextAlignment(HorizontalAlignment alignment);

    UIButton* GetButton() const { return mButton; }
    void SetPressCallback(const std::function<void()>& callback) { mPressCallback = callback; }
    void SetPressAudio(Audio* audio) { mPressAudio = audio; }
    void Press();

    bool IsAnimating() const { return mPressAnimTimer > 0.0f; }

    void SetSelected(bool selected) { mSelected = selected; }

protected:
    void OnUpdate(float deltaTime) override;

private:
    // Text label on the button.
    UILabel* mLabel = nullptr;

    // Fonts to use when button is interactable and not.
    Font* mFont = nullptr;
    Font* mDisabledFont = nullptr;

    // Colored border around the button.
    UINineSlice* mBorder = nullptr;

    // Clickable/hoverable area of the button.
    UIButton* mButton = nullptr;

    // When you press this button, the callback doesn't occur right away.
    // A short "blink" anim with some SFX plays, and then the callback executes.
    float mPressAnimTimer = 0.0f;
    std::function<void()> mPressCallback;

    // The audio to play when the button is pressed.
    // This is actually super inconsistent in the original game - unclear what the rhyme or reason is.
    Audio* mPressAudio = nullptr;

    // If a Sidney button is "selected", it means that it will stay highlighted gold, even after pressing it or no longer hovering it.
    // This is used to simulate "radio button group" style behavior.
    bool mSelected = false;
};
