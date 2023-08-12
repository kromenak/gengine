#include "ConsoleUI.h"

#include "AssetManager.h"
#include "Console.h"
#include "Font.h"
#include "Texture.h"
#include "UICanvas.h"
#include "UIImage.h"
#include "UILabel.h"
#include "UITextBuffer.h"
#include "UITextInput.h"
#include "Window.h"

ConsoleUI::ConsoleUI(bool mini) : Actor(TransformType::RectTransform),
	mMini(mini)
{
	// Add canvas for UI rendering. Draws above most other stuff.
	AddComponent<UICanvas>(10);
	
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
		backgroundImage->SetTexture(gAssetManager.LoadTexture("MINISNAKY.BMP"));
	}
	else
	{
		backgroundImage->SetTexture(gAssetManager.LoadTexture("SNAKY.BMP"));
	}
	
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
		mBackgroundTransform->SetAnchorMin(Vector2(0.0f, 1.0f));
		mBackgroundTransform->SetAnchorMax(Vector2(1.0f, 1.0f));
		mBackgroundTransform->SetPivot(0.5f, 1.0f);
		mBackgroundTransform->SetSizeDelta(0.0f, 0.0f);

        // Create scrollback text area.
        {
            Actor* scrollbackActor = new Actor(TransformType::RectTransform);
            mScrollbackTransform = scrollbackActor->GetComponent<RectTransform>();
            mScrollbackTransform->SetParent(mBackgroundTransform);

            // Scrollback takes up big chunk of space above horizontal rule.
            mScrollbackTransform->SetAnchorMin(Vector2(0.0f, 1.0f));
            mScrollbackTransform->SetAnchorMax(Vector2(1.0f, 1.0f));
            mScrollbackTransform->SetSizeDelta(-10.0f, 0.0f);
            mScrollbackTransform->SetPivot(0.5f, 1.0f);
            mScrollbackTransform->SetAnchoredPosition(0.0f, -kPaddingAboveScrollback);

            mScrollbackBuffer = scrollbackActor->AddComponent<UITextBuffer>();
            mScrollbackBuffer->SetFont(gAssetManager.LoadFont("F_CONSOLE_DISPLAY"));
        }

		// Add horizontal rule for full console.
		{
			mHorizontalRuleActor = new Actor(TransformType::RectTransform);
			RectTransform* hrTransform = mHorizontalRuleActor->GetComponent<RectTransform>();
			hrTransform->SetParent(mBackgroundTransform);
			
			// Horizontal rule uses a tiling line image.
			mHorizontalRuleActor->AddComponent<UIImage>();
			
			// Anchor along bottom edge of the console, with enough space for the input line below.
			hrTransform->SetAnchorMin(Vector2(0.0f, 0.0f));
			hrTransform->SetAnchorMax(Vector2(1.0f, 0.0f));
			hrTransform->SetSizeDelta(-8.0f, 1.0f);
			hrTransform->SetAnchoredPosition(0.0f, kHorizontalRuleOffsetFromBottom);
		}
		
		// Create text input field.
		{
			Font* font = gAssetManager.LoadFont("F_CONSOLE_COMMAND");
			
			Actor* textInputActor = new Actor(TransformType::RectTransform);
			RectTransform* textInputRT = textInputActor->GetComponent<RectTransform>();
			textInputRT->SetParent(mBackgroundTransform);
			
			// Input field takes up a single line below horizontal rule.
			textInputRT->SetAnchorMin(Vector2(0.0f, 0.0f));
			textInputRT->SetAnchorMax(Vector2(1.0f, 0.0f));
			textInputRT->SetSizeDelta(0.0f, static_cast<float>(font->GetGlyphHeight()));
			textInputRT->SetAnchoredPosition(4.0f, 5.0f);
            textInputRT->SetPivot(0.0f, 0.0f);
			
			mTextInput = textInputActor->AddComponent<UITextInput>();

			mTextInput->SetFont(font);
			mTextInput->SetText("");
			
			// Create text input field caret.
			Actor* caretActor = new Actor(TransformType::RectTransform);
			RectTransform* caretRT = caretActor->GetComponent<RectTransform>();
			caretRT->SetParent(textInputRT);
			
			// Horizontal rule uses a tiling line image.
			UIImage* caretImage = caretActor->AddComponent<UIImage>();
            caretImage->SetTexture(&Texture::White);

			caretRT->SetAnchorMin(Vector2(0.0f, 0.0f));
			caretRT->SetAnchorMax(Vector2(0.0f, 1.0f));
			caretRT->SetPivot(0.0f, 0.0f);
			caretRT->SetSizeDelta(1.0f, 4.0f);
			caretRT->SetAnchoredPosition(0.0f, 0.0f);
			
			mTextInput->SetCaret(caretImage);
			mTextInput->SetCaretBlinkInterval(0.5f);
		}
		
        // Create console activation image.
        {
            Actor* imageActor = new Actor(TransformType::RectTransform);
            RectTransform* imageRT = imageActor->GetComponent<RectTransform>();
            imageRT->SetParent(canvasTransform);

            mConsoleToggleImage = imageActor->AddComponent<UIImage>();

            Texture* texture = gAssetManager.LoadTexture("CAIN.BMP");
            Texture* textureAlpha = gAssetManager.LoadTexture("CAIN_ALPHA.BMP");
            texture->ApplyAlphaChannel(*textureAlpha);
            mConsoleToggleImage->SetTexture(texture, true);

            imageRT->SetAnchor(1.0f, 1.0f);
            imageRT->SetPivot(1.0f, 1.0f);
            imageRT->SetAnchoredPosition(-20.0f, -15.0f);
            mConsoleToggleImage->SetEnabled(false);
        }

        // Do a refresh so the initial state is correct.
        Refresh();
	}
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
        mConsoleToggleImage->SetEnabled(gInputManager.IsKeyPressed(SDL_SCANCODE_GRAVE));
		
		// On release, toggle the display of the console.
		if(gInputManager.IsKeyTrailingEdge(SDL_SCANCODE_GRAVE))
		{
            mConsoleActive = !mConsoleActive;
            Refresh();

            // Focus text field automatically on activate.
            // Also reset command history on show.
			if(mConsoleActive)
			{
				mTextInput->Focus();
				mCommandHistoryIndex = -1;
			}
			else
			{
				mTextInput->Unfocus();
			}
		}
		
		// Don't bother with other console updates unless it's opened.
		if(!mConsoleActive) { return; }
		
		// If enter is pressed, execute command in the input window.
		if(gInputManager.IsKeyLeadingEdge(SDL_SCANCODE_RETURN))
		{
            // Note that it is valid to execute an empty text input, which just outputs a dashed line to the console.
			gConsole.ExecuteCommand(mTextInput->GetText());
			mTextInput->Clear();
			
			// Executing a command resets command history.
			mCommandHistoryIndex = -1;
		}
		
		// Alt plus other keys affect the size of the full console.
		if(gInputManager.IsKeyPressed(SDL_SCANCODE_LALT) || gInputManager.IsKeyPressed(SDL_SCANCODE_RALT))
		{
			// Alt+Down increases console size by one line.
			if(gInputManager.IsKeyLeadingEdge(SDL_SCANCODE_DOWN))
			{
                RefreshMaxScrollbackLineCount();
				if(mScrollbackLineCount < mMaxScrollbackLineCount)
				{
					++mScrollbackLineCount;
					Refresh();
				}
			}
			// Alt+Up decreases console size by one line.
			else if(gInputManager.IsKeyLeadingEdge(SDL_SCANCODE_UP))
			{
				if(mScrollbackLineCount > 0)
				{
					--mScrollbackLineCount;
					Refresh();
				}
			}
			// Alt+PgDown adds 10 lines.
			else if(gInputManager.IsKeyLeadingEdge(SDL_SCANCODE_PAGEDOWN))
			{
                RefreshMaxScrollbackLineCount();
				mScrollbackLineCount += 10;
				if(mScrollbackLineCount > mMaxScrollbackLineCount)
				{
					mScrollbackLineCount = mMaxScrollbackLineCount;
				}
				Refresh();
			}
			// Alt+PgUp removes 10 lines.
			else if(gInputManager.IsKeyLeadingEdge(SDL_SCANCODE_PAGEUP))
			{
                if(mScrollbackLineCount < 10)
                {
                    mScrollbackLineCount = 0;
                }
                else
                {
                    mScrollbackLineCount -= 10;
                }
				Refresh();
			}
			// Alt+Home hides all lines.
			else if(gInputManager.IsKeyLeadingEdge(SDL_SCANCODE_HOME))
			{
				mScrollbackLineCount = 0;
				Refresh();
			}
			// Alt+End shows max number of lines.
			else if(gInputManager.IsKeyLeadingEdge(SDL_SCANCODE_END))
			{
                RefreshMaxScrollbackLineCount();
				mScrollbackLineCount = mMaxScrollbackLineCount;
				Refresh();
			}
		}
		// Ctrl plus other keys affect the position within the scrollback buffer.
		else if(gInputManager.IsKeyPressed(SDL_SCANCODE_LCTRL) || gInputManager.IsKeyPressed(SDL_SCANCODE_RCTRL))
		{
			// Ctrl+Down moves scrollback down one line.
			if(gInputManager.IsKeyLeadingEdge(SDL_SCANCODE_DOWN))
			{
				if(mScrollbackOffset > 0)
				{
					--mScrollbackOffset;
					Refresh();
				}
			}
			// Ctrl+Up moves scrollback up one line.
			else if(gInputManager.IsKeyLeadingEdge(SDL_SCANCODE_UP))
			{
                // To scroll up, the # of lines in the scrollback must be larger than what can fit on screen.
                // The "+1" here accounts for the horizontal rule, which counts as a line in the scrollback.
                if(gConsole.GetScrollback().size() + 1 > mScrollbackLineCount)
                {
                    uint32_t lastLineOffset = mScrollbackOffset + mScrollbackLineCount;
                    if(lastLineOffset <= gConsole.GetScrollback().size())
                    {
                        mScrollbackOffset++;
                        Refresh();
                    }
                }
			}
			// Ctrl+PgDown moves scrollback down one page worth of lines.
			else if(gInputManager.IsKeyLeadingEdge(SDL_SCANCODE_PAGEDOWN))
			{
                if(mScrollbackOffset > mScrollbackLineCount)
                {
                    mScrollbackOffset -= mScrollbackLineCount;
                }
                else
                {
                    mScrollbackOffset = 0;
                }
                Refresh();
			}
			// Ctrl+PgUp moves scrollback up one page worth of lines.
			else if(gInputManager.IsKeyLeadingEdge(SDL_SCANCODE_PAGEUP))
			{
                size_t scrollbackSize = gConsole.GetScrollback().size();
                if(scrollbackSize > 0 && scrollbackSize + 1 > mScrollbackLineCount)
                {
                    uint32_t maxOffset = scrollbackSize + 1 - mScrollbackLineCount;
                    mScrollbackOffset += mScrollbackLineCount;
                    if(mScrollbackOffset > maxOffset)
                    {
                        mScrollbackOffset = maxOffset;
                    }
                }
				Refresh();
			}
			// Ctrl+Home moves scrollback to earliest line.
			else if(gInputManager.IsKeyLeadingEdge(SDL_SCANCODE_HOME))
			{
                if(gConsole.GetScrollback().size() + 1 < mScrollbackLineCount)
                {
                    mScrollbackOffset = 0;
                }
                else
                {
                    mScrollbackOffset = gConsole.GetScrollback().size() + 1 - mScrollbackLineCount;
                }
				Refresh();
			}
			// Ctrl+End moves scrollback to the latest line.
			else if(gInputManager.IsKeyLeadingEdge(SDL_SCANCODE_END))
			{
				mScrollbackOffset = 0;
				Refresh();
			}
		}
        else
        {
            if(gInputManager.IsKeyLeadingEdge(SDL_SCANCODE_UP))
            {
                size_t historyLength = gConsole.GetCommandHistoryLength();
                if(historyLength > 0)
                {
                    // Increment command history index if it is in-bounds.
                    // First time this happens will go from -1 to 0, which is OK!
                    if(mCommandHistoryIndex < static_cast<int>(historyLength - 1))
                    {
                        ++mCommandHistoryIndex;
                    }

                    // Commands in history are earliest to most recent.
                    // So, when we push "up", we want to go to the next most recent item.
                    // Have to invert our index to get that.
                    int commandIndex = (historyLength - 1) - mCommandHistoryIndex;

                    // This changes the text... TODO: Better API for this?
                    mTextInput->Unfocus();
                    mTextInput->SetText(gConsole.GetCommandFromHistory(commandIndex));
                    mTextInput->Focus();
                }
            }
            else if(gInputManager.IsKeyLeadingEdge(SDL_SCANCODE_DOWN))
            {
                size_t historyLength = gConsole.GetCommandHistoryLength();
                if(historyLength > 0)
                {
                    // If current history index is above zero, decrement.
                    // Note that we can't decrement back to -1, per GK3 behavior.
                    if(mCommandHistoryIndex > 0)
                    {
                        --mCommandHistoryIndex;
                    }

                    // As above, need to invert the index.
                    int commandIndex = (historyLength - 1) - mCommandHistoryIndex;

                    // This changes the text... TODO: Better API for this?
                    mTextInput->Unfocus();
                    mTextInput->SetText(gConsole.GetCommandFromHistory(commandIndex));
                    mTextInput->Focus();
                }
            }
        }
	}
}

void ConsoleUI::Refresh()
{
    // Set entire thing active or not based on active boolean.
    mBackgroundTransform->GetOwner()->SetActive(mConsoleActive);

    // No need to update the whole console if inactive.
    if(!mConsoleActive)
    {
        return;
    }

    // HR is only enabled when scroll back is visible and positioned at very end (no offset).
    bool showHorizontalRule = mScrollbackLineCount > 0 && mScrollbackOffset == 0;
    mHorizontalRuleActor->SetActive(showHorizontalRule);

    // HR counts as one line of the scrollback, so the actual line count and offset depend on whether HR is visible.
    uint32_t lineCount = showHorizontalRule ? mScrollbackLineCount - 1 : mScrollbackLineCount;
    uint32_t offset = showHorizontalRule ? 0 : mScrollbackOffset - 1;
    mScrollbackBuffer->SetLineCount(lineCount);
    mScrollbackBuffer->SetLineOffset(offset);

    // Determine height of the scrollback buffer.
    // Each line has a certain height, and there are a certain number of lines.
    float scrollbackHeight = mScrollbackBuffer->CalculateHeight();
    mScrollbackTransform->SetSizeDeltaY(scrollbackHeight);
	
    // Determine height of the entire console.
    float height = kPaddingAboveScrollback + scrollbackHeight + CalcInputFieldHeight();
    if(showHorizontalRule)
    {
        height += mScrollbackBuffer->GetFont()->GetGlyphHeight() + 1;
    }
	mBackgroundTransform->SetSizeDeltaY(height);
}

void ConsoleUI::RefreshMaxScrollbackLineCount()
{
    // Calculate max number of lines in the console.
    float availableHeight = static_cast<float>(Window::GetHeight());
    availableHeight -= kPaddingAboveScrollback;
    availableHeight -= CalcInputFieldHeight();
    mMaxScrollbackLineCount = static_cast<int>(availableHeight / (mScrollbackBuffer->GetFont()->GetGlyphHeight() + 1));
}

float ConsoleUI::CalcInputFieldHeight() const
{
    // HR is positioned at 25 units. So, everything below HR is 25 - half line height (since HR is considered a line).
	// HR offset constant specifies center of last scrollback line as offset from bottom of console rect.
	// HR is meant to take up space of one scrollback line, so subtract half scrollback line height to get height just for input field.
	if(mScrollbackBuffer != nullptr)
	{
		return kHorizontalRuleOffsetFromBottom - (mScrollbackBuffer->GetFont()->GetGlyphHeight() / 2.0f);
	}
	return kHorizontalRuleOffsetFromBottom;
}
