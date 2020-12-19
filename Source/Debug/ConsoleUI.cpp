//
// Console.cpp
//
// Clark Kromenaker
//
#include "ConsoleUI.h"

#include "UICanvas.h"
#include "UIImage.h"
#include "UILabel.h"
#include "UITextBuffer.h"
#include "UITextInput.h"

ConsoleUI::ConsoleUI(bool mini) : Actor(TransformType::RectTransform),
	mMini(mini)
{
	// Add canvas for UI rendering.
	mCanvas = AddComponent<UICanvas>();
	
	// Create screen-sized canvas.
	RectTransform* canvasTransform = GetComponent<RectTransform>();
	canvasTransform->SetSizeDelta(0.0f, 0.0f);
	canvasTransform->SetAnchorMin(Vector2::Zero);
	canvasTransform->SetAnchorMax(Vector2::One);
	
	// Create background image actor and save transform (so we can move it around).
	Actor* background = new Actor(TransformType::RectTransform);
	mBackgroundTransform = background->GetComponent<RectTransform>();
	mBackgroundTransform->SetParent(canvasTransform);
	
	// Add background image that is tiled.
	// Only difference between mini and full is a different background image.
	UIImage* backgroundImage = background->AddComponent<UIImage>();
	backgroundImage->SetRenderMode(UIImage::RenderMode::Tiled);
	if(mini)
	{
		backgroundImage->SetTexture(Services::GetAssets()->LoadTexture("MINISNAKY"));
	}
	else
	{
		backgroundImage->SetTexture(Services::GetAssets()->LoadTexture("SNAKY"));
	}
	mCanvas->AddWidget(backgroundImage);
	
	// Mini and full consoles have different anchoring properties.
	// Mini is a box that is anchored to a corner of the screen.
	// Full is a panel that fully covers the top or bottom parts of the screen.
	if(mini)
	{
		
	}
	else
	{
		// Anchored along top edge of parent rect (anchor min/max).
		// Grow from top down (pivot).
		// No padding on left/right (sizeDelta.x).
		// Default height of 200 (sizeDelta.y).
		mBackgroundTransform->SetAnchorMin(Vector2(0.0f, 1.0f));
		mBackgroundTransform->SetAnchorMax(Vector2(1.0f, 1.0f));
		mBackgroundTransform->SetPivot(0.5f, 1.0f);
		mBackgroundTransform->SetSizeDelta(0.0f, 0.0f);
		
		// Add horizontal rule for full console.
		{
			mHorizontalRuleActor = new Actor(TransformType::RectTransform);
			RectTransform* hrTransform = mHorizontalRuleActor->GetComponent<RectTransform>();
			hrTransform->SetParent(mBackgroundTransform);
			
			// Horizontal rule uses a tiling line image.
			UIImage* hrImage = mHorizontalRuleActor->AddComponent<UIImage>();
			mCanvas->AddWidget(hrImage);
			
			// Anchor along bottom edge of the console, with enough space for the input line below.
			hrTransform->SetAnchorMin(Vector2(0.0f, 0.0f));
			hrTransform->SetAnchorMax(Vector2(1.0f, 0.0f));
			hrTransform->SetSizeDelta(-8.0f, 1.0f);
			hrTransform->SetAnchoredPosition(0.0f, kHorizontalRuleOffsetFromBottom);
		}
		
		// Create scrollback text area.
		{
			Actor* scrollbackActor = new Actor(TransformType::RectTransform);
			RectTransform* scrollbackRT = scrollbackActor->GetComponent<RectTransform>();
			scrollbackRT->SetParent(mBackgroundTransform);
			mScrollbackTransform = scrollbackRT;
			
			// Scrollback takes up big chunk of space above horizontal rule.
			scrollbackRT->SetAnchorMin(Vector2(0.0f, 1.0f));
			scrollbackRT->SetAnchorMax(Vector2(1.0f, 1.0f));
			scrollbackRT->SetSizeDelta(-20.0f, 0.0f);
			scrollbackRT->SetPivot(0.5f, 1.0f);
			
			mScrollbackBuffer = scrollbackActor->AddComponent<UITextBuffer>();
			mCanvas->AddWidget(mScrollbackBuffer);
			
			Font* font = Services::GetAssets()->LoadFont("F_CONSOLE_DISPLAY");
			mScrollbackBuffer->SetFont(font);
		}
		
		// Create text input field.
		{
			Font* font = Services::GetAssets()->LoadFont("F_CONSOLE_COMMAND");
			
			Actor* textInputActor = new Actor(TransformType::RectTransform);
			RectTransform* textInputRT = textInputActor->GetComponent<RectTransform>();
			textInputRT->SetParent(mBackgroundTransform);
			
			// Input field takes up a single line below horizontal rule.
			textInputRT->SetAnchorMin(Vector2(0.0f, 0.0f));
			textInputRT->SetAnchorMax(Vector2(1.0f, 0.0f));
			textInputRT->SetPivot(0.0f, 0.0f);
			textInputRT->SetSizeDelta(0.0f, font->GetGlyphHeight());
			textInputRT->SetAnchoredPosition(4.0f, 4.0f);
			
			mTextInput = textInputActor->AddComponent<UITextInput>();
			mCanvas->AddWidget(mTextInput);
			
			mTextInput->SetFont(font);
			mTextInput->SetText("");
			
			// Create text input field caret.
			Actor* caretActor = new Actor(TransformType::RectTransform);
			RectTransform* caretRT = caretActor->GetComponent<RectTransform>();
			caretRT->SetParent(textInputRT);
			
			// Horizontal rule uses a tiling line image.
			UIImage* caretImage = caretActor->AddComponent<UIImage>();
			mCanvas->AddWidget(caretImage);
			
			caretRT->SetAnchorMin(Vector2(0.0f, 0.0f));
			caretRT->SetAnchorMax(Vector2(0.0f, 1.0f));
			caretRT->SetPivot(0.0f, 0.0f);
			caretRT->SetSizeDelta(1.0f, 4.0f);
			caretRT->SetAnchoredPosition(0.0f, 0.0f);
			
			mTextInput->SetCaret(caretImage);
			mTextInput->SetCaretBlinkInterval(0.5f);
		}
		
		// Calculate max number of lines in the console.
		//TODO: Need to recalculate this if the screen resolution changes.
		float availableHeight = Services::GetRenderer()->GetWindowHeight();
		availableHeight -= CalcInputFieldHeight();
		mMaxScrollbackLineCount = availableHeight / mScrollbackBuffer->GetFont()->GetGlyphHeight();
	}
	
	// Disable canvas so console is hidden by default.
	mCanvas->SetEnabled(false);
}

void ConsoleUI::OnUpdate(float deltaTime)
{
	if(mMini)
	{
		//TODO: Mini console stuff!
		//TODO: Some key combo swaps mini console between the four corners of the screen
	}
	else
	{
		// Show an indicator when the console key is pressed down, but not yet released.
		if(Services::GetInput()->IsKeyPressed(SDL_SCANCODE_GRAVE))
		{
			//TODO: Show "Cain" image.
		}
		
		// On release, toggle the display of the console.
		if(Services::GetInput()->IsKeyUp(SDL_SCANCODE_GRAVE))
		{
			mCanvas->SetEnabled(!mCanvas->IsEnabled());
			if(mCanvas->IsEnabled())
			{
				Refresh();
				mTextInput->Clear();
				mTextInput->Focus();
				mCommandHistoryIndex = -1;
			}
			else
			{
				mTextInput->Unfocus();
			}
		}
		
		// Don't bother with other console updates unless it's opened.
		if(!mCanvas->IsActiveAndEnabled()) { return; }
		
		// If enter is pressed, execute sheep!
		if(Services::GetInput()->IsKeyDown(SDL_SCANCODE_RETURN))
		{
			Services::GetConsole()->ExecuteCommand(mTextInput->GetText());
			mTextInput->Clear();
			
			// Executing a command resets command history.
			mCommandHistoryIndex = -1;
		}
		
		// Alt plus other keys affect the size of the full console.
		if(Services::GetInput()->IsKeyPressed(SDL_SCANCODE_LALT))
		{
			// Alt+Down increases console size by one line.
			if(Services::GetInput()->IsKeyDown(SDL_SCANCODE_DOWN))
			{
				if(mScrollbackLineCount < mMaxScrollbackLineCount)
				{
					++mScrollbackLineCount;
					Refresh();
				}
			}
			// Alt+Up decreases console size by one line.
			else if(Services::GetInput()->IsKeyDown(SDL_SCANCODE_UP))
			{
				if(mScrollbackLineCount > 0)
				{
					--mScrollbackLineCount;
					Refresh();
				}
			}
			// Alt+PgDown adds 10 lines.
			else if(Services::GetInput()->IsKeyDown(SDL_SCANCODE_PAGEDOWN))
			{
				mScrollbackLineCount += 10;
				if(mScrollbackLineCount > mMaxScrollbackLineCount)
				{
					mScrollbackLineCount = mMaxScrollbackLineCount;
				}
				Refresh();
			}
			// Alt+PgUp removes 10 lines.
			else if(Services::GetInput()->IsKeyDown(SDL_SCANCODE_PAGEUP))
			{
				mScrollbackLineCount -= 10;
				if(mScrollbackLineCount < 0)
				{
					mScrollbackLineCount = 0;
				}
				Refresh();
			}
			// Alt+Home hides all lines.
			else if(Services::GetInput()->IsKeyDown(SDL_SCANCODE_HOME))
			{
				mScrollbackLineCount = 0;
				Refresh();
			}
			// Alt+End shows max number of lines.
			else if(Services::GetInput()->IsKeyDown(SDL_SCANCODE_END))
			{
				mScrollbackLineCount = mMaxScrollbackLineCount;
				Refresh();
			}
		}
		
		// Ctrl plus other keys affect the position within the scrollback buffer.
		if(Services::GetInput()->IsKeyPressed(SDL_SCANCODE_LCTRL))
		{
			// Ctrl+Down moves scrollback down one line.
			if(Services::GetInput()->IsKeyDown(SDL_SCANCODE_DOWN))
			{
				if(mScrollbackOffset > 0)
				{
					--mScrollbackOffset;
					Refresh();
				}
			}
			// Ctrl+Up moves scrollback up one line.
			else if(Services::GetInput()->IsKeyDown(SDL_SCANCODE_UP))
			{
				//TODO
			}
			// Ctrl+PgDown moves scrollback down 10 lines.
			else if(Services::GetInput()->IsKeyDown(SDL_SCANCODE_PAGEDOWN))
			{
				//TODO
				Refresh();
			}
			// Ctrl+PgUp moves scrollback up 10 lines.
			else if(Services::GetInput()->IsKeyDown(SDL_SCANCODE_PAGEUP))
			{
				//TODO
				Refresh();
			}
			// Ctrl+Home moves scrollback to earliest line.
			else if(Services::GetInput()->IsKeyDown(SDL_SCANCODE_HOME))
			{
				//TODO
				Refresh();
			}
			// Ctrl+End moves scrollback to the latest line.
			else if(Services::GetInput()->IsKeyDown(SDL_SCANCODE_END))
			{
				mScrollbackOffset = 0;
				Refresh();
			}
		}
		
		if(Services::GetInput()->IsKeyDown(SDL_SCANCODE_UP))
		{
			// Increment command history index if it is in-bounds.
			// First time this happens will go from -1 to 0, which is OK!
			int historyLength = Services::GetConsole()->GetCommandHistoryLength();
			if(mCommandHistoryIndex < historyLength - 1)
			{
				++mCommandHistoryIndex;
			}
			
			// Commands in history are earliest to most recent.
			// So, when we push "up", we want to go to the next most recent item.
			// Have to invert our index to get that.
			int commandIndex = (historyLength - 1) - mCommandHistoryIndex;
			std::string command = Services::GetConsole()->GetCommandFromHistory(commandIndex);
			
			// This changes the text... TODO: Better API for this?
			mTextInput->Unfocus();
			mTextInput->SetText(command);
			mTextInput->Focus();
		}
		else if(Services::GetInput()->IsKeyDown(SDL_SCANCODE_DOWN))
		{
			// If current history index is above zero, decrement.
			// Note that we can't decrement back to -1, per GK3 behavior.
			if(mCommandHistoryIndex > 0)
			{
				--mCommandHistoryIndex;
			}
			
			// As above, need to invert the index.
			int historyLength = Services::GetConsole()->GetCommandHistoryLength();
			int commandIndex = (historyLength - 1) - mCommandHistoryIndex;
			std::string command = Services::GetConsole()->GetCommandFromHistory(commandIndex);
			
			// This changes the text... TODO: Better API for this?
			mTextInput->Unfocus();
			mTextInput->SetText(command);
			mTextInput->Focus();
		}
	}
}

void ConsoleUI::Refresh()
{
	mScrollbackBuffer->SetLineOffset(mScrollbackOffset);
	mScrollbackBuffer->SetLineCount(mScrollbackLineCount);
	
	// HR is only enabled when line count is more than zero.
	//TODO: HR is only enabled when buffer pos is at bottom.
	mHorizontalRuleActor->SetActive(mScrollbackLineCount > 0);
	
	// Calculate how many scrollback lines we actually show.
	// HR counts as a line, so it takes some space.
	int lineCount = mScrollbackLineCount;
	if(mHorizontalRuleActor->IsActive())
	{
		--lineCount;
	}
	
	// Calculate size of one line of scrollback buffer.
	float scrollbackLineHeight = mScrollbackBuffer->GetFont()->GetGlyphHeight();
	
	// HR is positioned at 25 units. So, everything below HR is 25 - half line height (since HR is considered a line).
	float height = CalcInputFieldHeight();
	
	// If HR is active, it takes up some space.
	if(mHorizontalRuleActor->IsActive())
	{
		height += scrollbackLineHeight;
	}
	
	// Each line of the scrollback takes up some space.
	float scrollbackHeight = lineCount * scrollbackLineHeight;
	height += scrollbackHeight;
	
	// Set height for both the entire background and the scrollback buffer.
	mScrollbackTransform->SetSizeDeltaY(scrollbackHeight);
	mBackgroundTransform->SetSizeDeltaY(height);
	
	// FOR TESTING
	std::string bufferText;
	for(int i = 0; i < lineCount; ++i)
	{
		if(i == 2)
		{
			bufferText += "This is a really really really long line of text. It goes on and on and on. This is some really verbose console output text. Perhaps someone should consider editing this copy to make it shorter.\n";
		}
		else
		{
			bufferText += "Line" + std::to_string(i) + "\n";
		}
	}
	mScrollbackBuffer->SetText(bufferText);
}

float ConsoleUI::CalcInputFieldHeight() const
{
	// HR offset constant specifies center of last scrollback line as offset from bottom of console rect.
	// HR is meant to take up space of one scrollback line, so subtract half scrollback line height to get height just for input field.
	if(mScrollbackBuffer != nullptr)
	{
		return kHorizontalRuleOffsetFromBottom - (mScrollbackBuffer->GetFont()->GetGlyphHeight() / 2.0f);
	}
	return kHorizontalRuleOffsetFromBottom;
}
