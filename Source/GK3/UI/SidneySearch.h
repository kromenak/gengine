#pragma once

class Actor;
class UICanvas;
class UITextInput;

class SidneySearch
{
public:
    void Init(UICanvas* canvas);

    void Show();
    void Hide();

private:
    // Root of this screen.
    Actor* mRoot = nullptr;

    // Text input field.
    UITextInput* mTextInput = nullptr;
};