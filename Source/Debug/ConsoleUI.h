//
// Clark Kromenaker
//
// In-game console where some log data is output and sheep commands can be entered.
//
#pragma once
#include "Actor.h"

class RectTransform;
class UIImage;
class UILabel;
class UITextBuffer;
class UITextInput;

class ConsoleUI : public Actor
{
public:
	ConsoleUI(bool mini);
	
protected:
	void OnUpdate(float deltaTime) override;
	
private:
	// HR's position from bottom of the console.
	// HR signifies the CENTER of the last line of the scrollback buffer.
	// Everything below it is reserved for command input field.
	const float kHorizontalRuleOffsetFromBottom = 25.0f;

    // Extra space at top of console (above scrollback) for spacing.
    const float kPaddingAboveScrollback = 10.0f;
	
	// Is this a mini console?
	bool mMini = false;
	
	// The number of scrollback lines we want to show at one time.
	uint32_t mScrollbackLineCount = 10;
	
	// Max number of scrollback lines we can show.
    uint32_t mMaxScrollbackLineCount = 20;
	
	// Current scrollback offset.
    uint32_t mScrollbackOffset = 0;
	
	// Current history index. -1 means 'not at any index'.
	int mCommandHistoryIndex = -1;

    // Is console active?
    bool mConsoleActive = false;
    
	// Background and scrollback transforms, resized based on some keyboard shortcuts.
	RectTransform* mBackgroundTransform = nullptr;
	RectTransform* mScrollbackTransform = nullptr;

	// Horizontal rule actor is enabled/disabled in some circumstances.
	Actor* mHorizontalRuleActor = nullptr;
	
	// Scrollback buffer, contains console output text.
	UITextBuffer* mScrollbackBuffer = nullptr;
	
	// Input field, contains command text.
	UITextInput* mTextInput = nullptr;

    // Image that appears when about to activate console.
    // Doesn't really have any functional purpose.
    UIImage* mConsoleToggleImage = nullptr;
	
	void Refresh();
    void RefreshMaxScrollbackLineCount();
    
	float CalcInputFieldHeight() const;
};
