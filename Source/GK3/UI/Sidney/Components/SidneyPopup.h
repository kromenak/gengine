#pragma once
#include "Actor.h"

#include <functional>
#include <string>

#include "UILabel.h"

class SidneyButton;
class Texture;
class UIImage;

class SidneyPopup : public Actor
{
public:
    SidneyPopup(Actor* parent);

    void ResetToDefaults();

    void SetWindowPosition(const Vector2& position);

    void SetText(const std::string& message);
    void SetTextAlignment(HorizontalAlignment textAlignment);

    void SetImage(Texture* texture);

    void ShowOneButton();
    void ShowTwoButton(const std::function<void()>& yesCallback);

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
};