//
// Clark Kromenaker
//
// A popup in the Sidney UI. Supports many popup variations.
//
#pragma once
#include "Actor.h"

#include <functional>
#include <string>

#include "UILabel.h"

class SidneyButton;
class Texture;
class UIImage;
class UITextInput;

class SidneyPopup : public Actor
{
public:
    SidneyPopup(Actor* parent);

    void ResetToDefaults();

    void SetWindowPosition(const Vector2& position);
    void SetWindowSize(const Vector2& size);

    void SetText(const std::string& message);
    void SetTextAlignment(HorizontalAlignment textAlignment);

    void SetImage(Texture* texture);

    void ShowNoButton();
    void ShowOneButton();
    void ShowTwoButton(const std::function<void()>& yesCallback);
    void ShowThreeButton(const std::function<void(int)>& callback);
    void ShowTextInput(const std::function<void(const std::string&)>& submitCallback);

    void Hide();

protected:
    void OnUpdate(float deltaTime) override;

private:
    // Popup window; contains the content and buttons within the larger screen area.
    Actor* mWindow = nullptr;

    // A message displayed in the popup window.
    UILabel* mMessage = nullptr;

    // An image that is optionally displayed above the message.
    UIImage* mImage = nullptr;

    // For one button variant, the button to press to confirm message.
    SidneyButton* mOKButton = nullptr;

    // For the two button variant, yes and no buttons to press.
    SidneyButton* mYesButton = nullptr;
    SidneyButton* mNoButton = nullptr;

    // For the three button variant...
    SidneyButton* mLeftButton = nullptr;
    SidneyButton* mCenterButton = nullptr;
    SidneyButton* mRightButton = nullptr;

    // For the text input version of the popup, a text input field.
    UITextInput* mTextInput = nullptr;

    // A callback for when the text input is submitted.
    std::function<void(const std::string&)> mTextInputSubmitCallback = nullptr;
};