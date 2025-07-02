//
// Clark Kromenaker
//
// This popup shows a message with yes/no buttons.
//
#pragma once
#include "Actor.h"

#include <functional>

#include "LayerManager.h"

class RectTransform;
class UIButton;
class UILabel;

class ConfirmPopup : public Actor
{
public:
    ConfirmPopup();

    void Show(const std::string& message, const std::function<void(bool)>& callback);
    void Hide();

protected:
    void OnUpdate(float deltaTime) override;

private:
    Layer mLayer;

    // The transform for the popup box.
    RectTransform* mPopupTransform = nullptr;

    // The amount the text is offset from the top of the box.
    float mTextOffsetY = 0.0f;

    // The label that shows whatever message has been passed in.
    UILabel* mMessageLabel = nullptr;

    // The two buttons that can be pressed.
    UIButton* mYesButton = nullptr;
    UIButton* mNoButton = nullptr;

    // A callback to call when the user has pressed the yes/no buttons.
    std::function<void(bool)> mCallback = nullptr;

    void OnYesButtonPressed();
    void OnNoButtonPressed();
};