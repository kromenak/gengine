#include "SidneyAnagramParser.h"

#include "ActionManager.h"
#include "AssetManager.h"
#include "GameProgress.h"
#include "Random.h"
#include "SidneyButton.h"
#include "SidneyUtil.h"
#include "UIButton.h"
#include "UICanvas.h"
#include "UIImage.h"
#include "UILabel.h"
#include "UINineSlice.h"
#include "UIScrollRect.h"
#include "UIUtil.h"

SidneyAnagramParser::SidneyAnagramParser(Actor* parent) : Actor("Anagram Parser", TransformType::RectTransform)
{
    GetTransform()->SetParent(parent->GetTransform());

    // Create overall window with border.
    UINineSlice* window = AddComponent<UINineSlice>(SidneyUtil::GetGrayBoxParams(SidneyUtil::TransBgColor));
    window->GetRectTransform()->SetSizeDelta(600.0f, 319.0f);

    // Lay out divider lines for the various areas of the anagram parser window.
    {
        UIImage* dividerImage = UI::CreateWidgetActor<UIImage>("Divider", window);
        dividerImage->SetTexture(gAssetManager.LoadTexture("S_BOX_TOP.BMP"), true);
        dividerImage->GetRectTransform()->SetAnchor(AnchorPreset::TopStretch);
        dividerImage->GetRectTransform()->SetAnchoredPosition(0.0f, -61.0f);
        dividerImage->GetRectTransform()->SetSizeDeltaX(0.0f);

        dividerImage = UI::CreateWidgetActor<UIImage>("Divider", window);
        dividerImage->SetTexture(gAssetManager.LoadTexture("S_BOX_TOP.BMP"), true);
        dividerImage->GetRectTransform()->SetAnchor(AnchorPreset::TopStretch);
        dividerImage->GetRectTransform()->SetAnchoredPosition(0.0f, -231.0f);
        dividerImage->GetRectTransform()->SetSizeDeltaX(0.0f);

        dividerImage = UI::CreateWidgetActor<UIImage>("Divider", window);
        dividerImage->SetTexture(gAssetManager.LoadTexture("S_BOX_TOP.BMP"), true);
        dividerImage->GetRectTransform()->SetAnchor(AnchorPreset::TopStretch);
        dividerImage->GetRectTransform()->SetAnchoredPosition(0.0f, -291.0f);
        dividerImage->GetRectTransform()->SetSizeDeltaX(0.0f);

        dividerImage = UI::CreateWidgetActor<UIImage>("Divider", window);
        dividerImage->SetTexture(gAssetManager.LoadTexture("S_BOX_SIDE.BMP"), true);
        dividerImage->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
        dividerImage->GetRectTransform()->SetAnchoredPosition(183.0f, -61.0f);
        dividerImage->GetRectTransform()->SetSizeDelta(2.0f, 171.0f);
    }

    // Create labels in the header area.
    {
        UILabel* titleLabel = UI::CreateWidgetActor<UILabel>("TitleLabel", window);
        titleLabel->SetFont(gAssetManager.LoadFont("SID_TEXT_14.FON"));
        titleLabel->SetText(SidneyUtil::GetAnalyzeLocalizer().GetText("AnagramTitle"));
        titleLabel->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
        titleLabel->GetRectTransform()->SetAnchoredPosition(15.0f, -9.0f);
        titleLabel->FitRectTransformToText();

        mParsingHeaderLabel = UI::CreateWidgetActor<UILabel>("ParsingLabel", window);
        mParsingHeaderLabel->SetFont(gAssetManager.LoadFont("SID_TEXT_14.FON"));
        mParsingHeaderLabel->SetText(SidneyUtil::GetAnalyzeLocalizer().GetText("Parsing"));
        mParsingHeaderLabel->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
        mParsingHeaderLabel->GetRectTransform()->SetAnchoredPosition(161.0f, -9.0f);
        mParsingHeaderLabel->FitRectTransformToText();

        const float kLetterX = 88.0f;
        const float kLetterSpacing = 20.0f;
        for(int i = 0; i < kMaxAnagramLetters; ++i)
        {
            mLetterLabels[i] = UI::CreateWidgetActor<UILabel>("AnagramLetter" + std::to_string(i), window);
            mLetterLabels[i]->SetFont(gAssetManager.LoadFont("SID_TEXT_14.FON"));
            mLetterLabels[i]->SetText("A");
            mLetterLabels[i]->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
            mLetterLabels[i]->GetRectTransform()->SetAnchoredPosition(kLetterX + i * kLetterSpacing, -34.0f);
            mLetterLabels[i]->FitRectTransformToText();
        }
    }

    // Create scrollable "select" area to take letters from.
    {
        // Add scroll area
        {
            UICanvas* scrollCanvas = UI::CreateCanvas("WordSelectCanvas", window, 1, Color32::Black);
            scrollCanvas->SetMasked(true);
            scrollCanvas->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
            scrollCanvas->GetRectTransform()->SetAnchoredPosition(1.0f, -62.0f);
            scrollCanvas->GetRectTransform()->SetSizeDelta(182.0f, 169.0f);

            UIScrollRect* scrollRect = new UIScrollRect(scrollCanvas->GetOwner());
            scrollRect->GetRectTransform()->SetAnchor(AnchorPreset::CenterStretch);
            scrollRect->GetRectTransform()->SetSizeDelta(0.0f, 0.0f);
            scrollRect->SetScrollbarWidth(5.0f);
            mAvailableWordsScrollRect = scrollRect;

            for(int i = 0; i < kMaxAnagramWords; ++i)
            {
                UILabel* label = UI::CreateWidgetActor<UILabel>("AnagramWord" + std::to_string(i), scrollRect);
                label->SetFont(gAssetManager.LoadFont("SID_TEXT_14.FON"));
                label->SetText(SidneyUtil::GetAnalyzeLocalizer().GetText("Word" + std::to_string(i + 1)));
                label->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
                label->FitRectTransformToText();
                mAvailableWordLabels[i] = label;

                UIButton* button = label->GetOwner()->AddComponent<UIButton>();
                button->SetPressCallback([this, i](UIButton* button){
                    OnAvailableWordSelected(i);
                });
            }
            RefreshAvailableWordsScrollList();

            // Add header box
            UINineSlice* box = UI::CreateWidgetActor<UINineSlice>("HeaderBox", scrollCanvas, SidneyUtil::GetGrayBoxParams(Color32::Black));
            box->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
            box->GetRectTransform()->SetAnchoredPosition(-1.0f, 1.0f);
            box->GetRectTransform()->SetSizeDelta(60.0f, 14.0f);

            // Add header label.
            UILabel* label = UI::CreateWidgetActor<UILabel>("SelectLabel", box);
            label->SetFont(gAssetManager.LoadFont("SID_PDN_10_UL.FON"));
            label->SetText(SidneyUtil::GetAnalyzeLocalizer().GetText("SelectText"));
            label->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
            label->GetRectTransform()->SetAnchoredPosition(8.0f, -2.0f);
            label->FitRectTransformToText();
        }
    }

    // Create message area.
    {
        mMessagesLabel = UI::CreateWidgetActor<UILabel>("MessageLabel", window);
        mMessagesLabel->SetFont(gAssetManager.LoadFont("SID_TEXT_14.FON"));
        mMessagesLabel->SetText(SidneyUtil::GetAnalyzeLocalizer().GetText("LatinMsg") + "\n" + SidneyUtil::GetAnalyzeLocalizer().GetText("Latin2Msg"));
        mMessagesLabel->SetVerticalAlignment(VerticalAlignment::Top);
        mMessagesLabel->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
        mMessagesLabel->GetRectTransform()->SetAnchoredPosition(193.0f, -68.5f);
        mMessagesLabel->GetRectTransform()->SetSizeDelta(405.0f, 160.0f);
    }

    // Create "phrase building area."
    {
        // Add header box.
        UINineSlice* box = UI::CreateWidgetActor<UINineSlice>("HeaderBox", window, SidneyUtil::GetGrayBoxParams(Color32::Black));
        box->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
        box->GetRectTransform()->SetAnchoredPosition(0.0f, -231.0f);
        box->GetRectTransform()->SetSizeDelta(160.0f, 14.0f);

        // Add header label.
        UILabel* label = UI::CreateWidgetActor<UILabel>("PhraseBuildingLabel", box);
        label->SetFont(gAssetManager.LoadFont("SID_PDN_10_UL.FON"));
        label->SetText(SidneyUtil::GetAnalyzeLocalizer().GetText("PhraseText"));
        label->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
        label->GetRectTransform()->SetAnchoredPosition(8.0f, -2.0f);
        label->FitRectTransformToText();

        // Create "slots" for each selected word.
        const float kSlotSpacing = 50.0f;
        float slotX = 28.0f;
        for(int i = 0; i < kMaxSelectedWords; ++i)
        {
            UIImage* underline = UI::CreateWidgetActor<UIImage>("Slot" + std::to_string(i), window);
            underline->SetTexture(gAssetManager.LoadTexture("S_BOX_TOP.BMP"), true);
            underline->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
            underline->GetRectTransform()->SetAnchoredPosition(slotX + kSlotSpacing * i, -262.0f);
            underline->GetRectTransform()->SetSizeDelta(40.0f, 1.0f);

            mSelectedWordLabels[i] = UI::CreateWidgetActor<UILabel>("SlotWord" + std::to_string(i), window);
            mSelectedWordLabels[i]->SetFont(gAssetManager.LoadFont("SID_TEXT_14.FON"));
            mSelectedWordLabels[i]->SetText("");
            mSelectedWordLabels[i]->SetHorizonalAlignment(HorizontalAlignment::Center);
            mSelectedWordLabels[i]->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
            mSelectedWordLabels[i]->GetRectTransform()->SetAnchoredPosition(slotX + kSlotSpacing * i, -262.0f + 16.0f);
            mSelectedWordLabels[i]->GetRectTransform()->SetSizeDelta(40.0f, 15.0f);
        }

        mTranslatedTextLabel = UI::CreateWidgetActor<UILabel>("TranslatedTextLabel", window);
        mTranslatedTextLabel->SetFont(gAssetManager.LoadFont("SID_TEXT_14.FON"));
        mTranslatedTextLabel->SetText("");
        mTranslatedTextLabel->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
        mTranslatedTextLabel->GetRectTransform()->SetAnchoredPosition(slotX, -270.0f);
        mTranslatedTextLabel->GetRectTransform()->SetSizeDelta(100.0f, 15.0f);
    }

    // Create footer.
    {
        SidneyButton* eraseButton = SidneyUtil::CreateSmallButton(window->GetOwner());
        eraseButton->SetName("EraseButton");
        eraseButton->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
        eraseButton->GetRectTransform()->SetAnchoredPosition(10.0f, -299.0f);
        eraseButton->SetWidth(60.0f);
        eraseButton->SetText(SidneyUtil::GetAnalyzeLocalizer().GetText("EraseButton"));
        eraseButton->SetPressCallback([this](){
            OnEraseButtonPressed();
        });
        mEraseButton = eraseButton;

        SidneyButton* exitButton = SidneyUtil::CreateSmallButton(window->GetOwner());
        exitButton->SetName("ExitButton");
        exitButton->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
        exitButton->GetRectTransform()->SetAnchoredPosition(83.0f, -299.0f);
        exitButton->SetWidth(60.0f);
        exitButton->SetText(SidneyUtil::GetAnalyzeLocalizer().GetText("ExitButton"));
        exitButton->SetPressCallback([this](){
            Hide();
            if(mExitCallback != nullptr)
            {
                mExitCallback();
                mExitCallback = nullptr;
            }
        });
        mExitButton = exitButton;
    }

    // Hide the anagram parser to start.
    Hide();
}

void SidneyAnagramParser::Show(const std::string& anagramText, const std::function<void()>& exitCallback)
{
    // Already showing, don't run show code again.
    if(IsActive()) { return; }
    SetActive(true);

    // Start in "idle" state.
    mState = State::Idle;

    // Save exit callback.
    mExitCallback = exitCallback;

    // Populate the "Parsing: " text field.
    mParsingHeaderLabel->SetText(SidneyUtil::GetAnalyzeLocalizer().GetText("Parsing") + " " + anagramText);

    // The letters are the text without spaces, all uppercase.
    mAllLetters = anagramText;
    StringUtil::RemoveAll(mAllLetters, ' ');
    StringUtil::ToUpper(mAllLetters);

    // At this point, all letters are available.
    mRemainingLetters = mAllLetters;

    // Populate individual anagram parser letters.
    Font* font = gAssetManager.LoadFont("SID_TEXT_14.FON");
    for(int i = 0; i < kMaxAnagramLetters; ++i)
    {
        if(i < mAllLetters.length())
        {
            mLetterLabels[i]->SetEnabled(true);
            mLetterLabels[i]->SetText(std::string(1, mAllLetters[i]));
            mLetterLabels[i]->SetFont(font);
        }
        else
        {
            mLetterLabels[i]->SetEnabled(false);
        }
    }

    // Reset the available words list to its original state.
    // To do this, turn on all labels and then refresh the list (so all active labels are repositioned).
    for(int i = 0; i < kMaxAnagramWords; ++i)
    {
        mAvailableWordLabels[i]->GetOwner()->SetActive(true);
    }
    RefreshAvailableWordsScrollList();

    // Then turn all labels off, so they can be turned on when the parser is running.
    for(int i = 0; i < kMaxAnagramWords; ++i)
    {
        mAvailableWordLabels[i]->GetOwner()->SetActive(false);
    }

    // Clear all text from the message area.
    mMessagesLabel->SetText("");

    // Disable the erase/exit buttons until the scramble animation is over.
    mEraseButton->GetButton()->SetCanInteract(false);
    mExitButton->GetButton()->SetCanInteract(false);

    // Make sure selected word slots are reset.
    mSelectedWordIndexes.clear();
    for(int i = 0; i < kMaxSelectedWords; ++i)
    {
        mSelectedWordLabels[i]->SetText("");
    }
    mTranslatedTextLabel->SetText("");

    // Pause for dramatic effect; I guess Sidney is thinking about it...
    gActionManager.ExecuteSheepAction("wait SetTimerSeconds(2)", [this](const Action* action){

        // Show message that essentially says "searching for latin words in this phrase."
        AppendMessageText("LatinMsg");
        AppendMessageText("Latin2Msg");

        // Start the scramble effect.
        mState = State::InitialScramble;
        StartScramble();
    });
}

void SidneyAnagramParser::Hide()
{
    SetActive(false);
}

void SidneyAnagramParser::OnUpdate(float deltaTime)
{
    // Decrement timer until next scramble.
    if(mScrambleTimer > 0.0f)
    {
        mScrambleTimer -= deltaTime;
        if(mScrambleTimer <= 0.0f)
        {
            // Do the scramble!
            Font* unavailableFont = gAssetManager.LoadFont("SID_TEXT_14_UL.FON");
            std::string lettersRemaining = mRemainingLetters;
            for(int i = 0; i < kMaxAnagramLetters; ++i)
            {
                if(mLetterLabels[i]->GetFont() == unavailableFont) { continue; }

                int randomIndex = Random::RangeSize(0, lettersRemaining.length());
                mLetterLabels[i]->SetText(std::string(1, lettersRemaining[randomIndex]));
                lettersRemaining.erase(randomIndex, 1);
                if(lettersRemaining.empty())
                {
                    break;
                }
            }

            // Scramble again after X seconds have passed.
            mScrambleTimer = kScrambleInterval;
        }
    }

    // Decrement timer until add another discovered word.
    if(mScrambleAddWordTimer > 0.0f)
    {
        mScrambleAddWordTimer -= deltaTime;
        if(mScrambleAddWordTimer <= 0.0f)
        {
            mAvailableWordLabels[mCurrentWordsIndex]->GetOwner()->SetActive(true);
            mAvailableWordLabels[mCurrentWordsIndex]->SetText(SidneyUtil::GetAnalyzeLocalizer().GetText(mLocWordsPrefix + std::to_string(mCurrentWordsIndex + 1)));
            //printf("%d - %s\n", mCurrentWordsIndex, mAvailableWordLabels[mCurrentWordsIndex]->GetText().c_str());
            ++mCurrentWordsIndex;

            if(mCurrentWordsIndex < mWordsCount)
            {
                mScrambleAddWordTimer = kScrambleAddWordInterval;
            }
            else
            {
                StopScramble();
                mState = State::SelectWords;
            }
        }
    }

    // Decrement final scramble timer.
    if(mFinalScrambleTimer > 0.0f)
    {
        mFinalScrambleTimer -= deltaTime;
        if(mFinalScrambleTimer <= 0.0f)
        {
            // Stop the scramble.
            mState = State::Solved;
            StopScramble();

            // Start a manual action during the next little cutscene here...
            gActionManager.StartManualAction();

            // Show "we found a match" message, and wait a moment.
            AppendMessageText("MatchMsg");
            gActionManager.ExecuteSheepAction("wait SetTimerSeconds(2)", [this](const Action* action){

                // Populate the missing word.
                // Note that -1 is a special value that the refresh code corresponds to the missing word.
                mSelectedWordIndexes.push_back(-1);
                RefreshSelectedAndAvailableWords();

                // Again, wait a moment.
                gActionManager.ExecuteSheepAction("wait SetTimerSeconds(2)", [this](const Action* action){

                    // Display the final text with all selected words.
                    std::string text = StringUtil::Format(SidneyUtil::GetAnalyzeLocalizer().GetText("CorrectMsg").c_str(),
                                                          mSelectedWordLabels[0]->GetText().c_str(),
                                                          mSelectedWordLabels[1]->GetText().c_str(),
                                                          mSelectedWordLabels[2]->GetText().c_str());
                    AppendMessageText(text);

                    // Again, wait a moment.
                    gActionManager.ExecuteSheepAction("wait SetTimerSeconds(2)", [this](const Action* action){

                        // Another short message, and wait a moment...
                        AppendMessageText("RebusMsg");
                        gActionManager.ExecuteSheepAction("wait SetTimerSeconds(2)", [this](const Action* action){

                            // Show final translation.
                            AppendMessageText("TransMsg");
                            gActionManager.FinishManualAction();

                            // Grace says "oh my gawwwwwd".
                            gActionManager.ExecuteDialogueAction("02OFT2ZRNA", 1);

                            // We completed the puzzle.
                            gGameProgress.ChangeScore("e_sidney_analysis_complete_anagram");
                            gGameProgress.SetFlag("Ophiuchus");
                            SidneyUtil::UpdateLSRState();
                        });
                    });
                });
            });
        }
    }
}

void SidneyAnagramParser::AppendMessageText(const std::string& locKey)
{
    std::string messagesText = mMessagesLabel->GetText();
    if(!messagesText.empty())
    {
        messagesText += "\n";
    }
    messagesText += SidneyUtil::GetAnalyzeLocalizer().GetText(locKey);
    mMessagesLabel->SetText(messagesText);
}

bool SidneyAnagramParser::RefreshSelectedAndAvailableWords()
{
    // Let's start off by assuming that all available word labels are active.
    for(int i = 0; i < kMaxAnagramWords; ++i)
    {
        mAvailableWordLabels[i]->GetOwner()->SetActive(true);
    }

    // Iterate all selected word labels to update their text.
    // As we do that, we can also determine the translated text AND which letters have been used.
    std::string usedLetters;
    std::string translatedText;
    for(int i = 0; i < kMaxSelectedWords; ++i)
    {
        // This slot is beyond the number of selected words, so just leave it empty.
        if(i >= mSelectedWordIndexes.size())
        {
            mSelectedWordLabels[i]->SetText("");
            continue;
        }

        // Get the text for this selected word.
        int wordIndex = mSelectedWordIndexes[i];
        std::string labelText;
        if(wordIndex >= 0)
        {
            labelText = mAvailableWordLabels[wordIndex]->GetText();
        }
        else
        {
            // At the end of the anagram puzzle, a final word is added that isn't otherwise available/selectable.
            // To signify this, we'll use the special "-1" word index.
            labelText = SidneyUtil::GetAnalyzeLocalizer().GetText("FinalWord");
        }

        // The Latin word and the English translation are in a single string.
        // We need to divide them into separate strings, but using a space won't work (some of the words/translations have spaces in them).
        // Instead, use the open parentheses character, which always occurs ONLY at the start of the translation.
        std::vector<std::string> wordAndTranslation = StringUtil::Split(labelText, '(', true);
        if(wordAndTranslation.size() != 2)
        {
            printf("Unexpected format for word/translation combo in anagram parser: %s\n", labelText.c_str());
            continue;
        }

        // Get rid of any leading/trailing spaces.
        StringUtil::Trim(wordAndTranslation[0]);
        StringUtil::Trim(wordAndTranslation[1]);

        // The split above removed the open parentheses - manually remove the close one.
        wordAndTranslation[1].pop_back();

        // Put the selected Latin word in the selected word label.
        mSelectedWordLabels[i]->SetText(wordAndTranslation[0]);

        // Remember that these letters are used.
        usedLetters += wordAndTranslation[0];

        // Add translation to end of translated text.
        if(!translatedText.empty())
        {
            translatedText.push_back(' ');
        }
        translatedText += wordAndTranslation[1];

        // This selectable word label should definitely be disabled, since it's in one of the slots.
        if(wordIndex >= 0)
        {
            mAvailableWordLabels[wordIndex]->GetOwner()->SetActive(false);
        }
    }

    // Set the deciphered text label with what we've generated.
    mTranslatedTextLabel->SetText(translatedText);

    // Ok...now here's some fun! Based on the letters used so far, figure out what letters are remaining from the original letters.
    StringUtil::ToUpper(usedLetters);
    mRemainingLetters = mAllLetters;
    for(char c : usedLetters)
    {
        size_t index = mRemainingLetters.find(c);
        if(index != std::string::npos)
        {
            mRemainingLetters.erase(index, 1);
        }
    }
    //printf("Remaining letters: %s\n", mRemainingLetters.c_str());

    // AND NOW...go through every selectable word, and disable any one that can't be spelled with the remaining letters.
    for(int i = 0; i < kMaxAnagramWords; ++i)
    {
        // Skip already disabled ones.
        if(!mAvailableWordLabels[i]->IsActiveAndEnabled())
        {
            continue;
        }

        // Get the label and figure out where the Latin word ends (at the space before opening paretheses).
        const std::string& labelText = mAvailableWordLabels[i]->GetText();
        size_t wordEndIndex = labelText.find('(') - 1;

        // If the word has more letters than are remaining, we definitely can't spell it. Disable it.
        if(wordEndIndex > mRemainingLetters.size())
        {
            mAvailableWordLabels[i]->GetOwner()->SetActive(false);
            continue;
        }

        // There are enough letters remaining, but now we have to see if they're the RIGHT letters.
        // This is somewhat inefficient, but make a copy of the remaining letters, and go one-by-one to make sure we have all the needed letters.
        std::string letters = mRemainingLetters;
        for(int j = 0; j < wordEndIndex; ++j)
        {
            char upperChar = static_cast<char>(std::toupper(labelText[j]));
            size_t index = letters.find(upperChar);
            if(index != std::string::npos)
            {
                // If one of the word's letters list, remove it as used.
                letters.erase(index, 1);
            }
            else
            {
                // A letter was missing, so this word is out. We can also break at this point.
                mAvailableWordLabels[i]->GetOwner()->SetActive(false);
                break;
            }
        }
    }

    // We may have disabled many words in the available list.
    // Refresh the scroll list so all remaining words are listed next to one another.
    RefreshAvailableWordsScrollList();

    // Finally, refresh the remaining letters in the letter labels.
    // The easiest way to do this is to mark them all available, then go back and mark some unavailable.
    Font* availableFont = gAssetManager.LoadFont("SID_TEXT_14.FON");
    Font* unavailableFont = gAssetManager.LoadFont("SID_TEXT_14_UL.FON");
    for(int i = 0; i < kMaxAnagramLetters; ++i)
    {
        mLetterLabels[i]->SetFont(availableFont);
    }

    // For each remaining letter...
    for(int i = 0; i < usedLetters.length(); ++i)
    {
        // Find a label with that same letter, that is active/enabled, and has the available font...make it unavailable!
        for(int j = 0; j < kMaxAnagramLetters; ++j)
        {
            if(mLetterLabels[j]->IsActiveAndEnabled() &&
               mLetterLabels[j]->GetFont() == availableFont &&
               mLetterLabels[j]->GetText()[0] == usedLetters[i])
            {
                mLetterLabels[j]->SetFont(unavailableFont);
                break;
            }
        }
    }

    // Return whether there are any more words left to select.
    for(int i = 0; i < kMaxAnagramWords; ++i)
    {
        if(mAvailableWordLabels[i]->IsActiveAndEnabled())
        {
            return true;
        }
    }
    return false;
}

void SidneyAnagramParser::RefreshAvailableWordsScrollList()
{
    // Reset to top.
    mAvailableWordsScrollRect->SetNormalizedScrollValue(0.0f);

    // Reposition active words from the top, one after another.
    const float kSelectLabelX = 14.0f;
    const float kSelectLabelStartY = -23.0f;
    const float kSelectLabelSpacing = 15.0f;
    int activeWordIndex = 0;
    for(int i = 0; i < kMaxAnagramWords; ++i)
    {
        if(mAvailableWordLabels[i]->IsActiveAndEnabled())
        {
            mAvailableWordLabels[i]->GetRectTransform()->SetAnchoredPosition(kSelectLabelX, kSelectLabelStartY - (activeWordIndex * kSelectLabelSpacing));
            ++activeWordIndex;
        }
    }
}

void SidneyAnagramParser::StartScramble()
{
    // Play a "we're scrambling the letters" sound.
    PlayAudioParams audioParams;
    audioParams.audio = gAssetManager.LoadAudio("SIDANAGRAMSCRAMBLE.WAV", AssetScope::Scene);
    audioParams.audioType = AudioType::SFX;
    audioParams.loopCount = -1;
    mScrambleSoundHandle = gAudioManager.Play(audioParams);

    // Start the scramble pretty much right away.
    mScrambleTimer = 0.001f;

    // The game is non-interactive until the scramble completes.
    gActionManager.StartManualAction();

    // During initial scramble, words are added.
    if(mState == State::InitialScramble)
    {
        // The number of anagram words detected differs based on whether "SUM" has been added.
        if(gGameProgress.GetFlag("ArcadiaComplete"))
        {
            mLocWordsPrefix = "Word";
            mWordsCount = 161;
        }
        else
        {
            mLocWordsPrefix = "WordB";
            mWordsCount = 36;
        }

        // Make sure we start at the first word.
        mCurrentWordsIndex = 0;

        // There's an initial delay before words start being found/added from the parser.
        mScrambleAddWordTimer = kScrambleAddWordInitialDelay;
    }
    else if(mState == State::FinalScramble)
    {
        mFinalScrambleTimer = kFinalScrambleDuration;
    }
}

void SidneyAnagramParser::StopScramble()
{
    // Stop the scramble SFX.
    gAudioManager.Stop(mScrambleSoundHandle);

    // Stop the word scramble effect.
    for(int i = 0; i < kMaxAnagramLetters; ++i)
    {
        mLetterLabels[i]->SetText(std::string(1, mAllLetters[i]));
    }
    mScrambleTimer = 0.0f;

    // Show message indicating that all words have been found and now you have to choose them.
    if(mState == State::InitialScramble)
    {
        AppendMessageText("SelectMsg");
    }

    // Should be interactive again.
    gActionManager.FinishManualAction();

    // Enable the anagram exit button at this point.
    mExitButton->GetButton()->SetCanInteract(true);
}

void SidneyAnagramParser::OnAvailableWordSelected(int index)
{
    if(mSelectedWordIndexes.size() < kMaxSelectedWords)
    {
        // Add selected word.
        mSelectedWordIndexes.push_back(index);

        // Refresh the selected word slots, the list of available words, and the letters list.
        bool anyWordsLeft = RefreshSelectedAndAvailableWords();
        if(!anyWordsLeft)
        {
            // See if we've selected the correct words to solve the puzzle...
            const int kWord1Index = 12;
            const int kWord2Index = 40;
            const int kWord3Index = 146;
            if(mSelectedWordIndexes.size() == 3)
            {
                // The words can be in any order, so we just need to ensure they are contained in the list.
                auto it1 = std::find(mSelectedWordIndexes.begin(), mSelectedWordIndexes.end(), kWord1Index);
                auto it2 = std::find(mSelectedWordIndexes.begin(), mSelectedWordIndexes.end(), kWord2Index);
                auto it3 = std::find(mSelectedWordIndexes.begin(), mSelectedWordIndexes.end(), kWord3Index);
                if(it1 != mSelectedWordIndexes.end() && it2 != mSelectedWordIndexes.end() && it3 != mSelectedWordIndexes.end())
                {
                    // If all's well, the anagram is solved - show the final scramble and messages.
                    AppendMessageText("CheckingMsg");
                    mState = State::FinalScramble;
                    StartScramble();
                }
            }

            // If the state hasn't changed, the player didn't choose the correct words. Let them know.
            if(mState != State::FinalScramble)
            {
                gActionManager.StartManualAction();
                AppendMessageText("CheckingMsg");
                gActionManager.ExecuteSheepAction("wait SetTimerSeconds(2)", [this](const Action* action){
                    AppendMessageText("NoMatchMsg");
                    gActionManager.ExecuteSheepAction("wait SetTimerSeconds(2)", [this](const Action* action){
                        AppendMessageText("TryAgain");
                        mState = State::Unsolved;
                        gActionManager.FinishManualAction();
                    });
                });
            }
        }

        // Since we have selected words, the erase button should be active.
        mEraseButton->GetButton()->SetCanInteract(mState == State::SelectWords || mState == State::Unsolved);
    }
}

void SidneyAnagramParser::OnEraseButtonPressed()
{
    if(!mSelectedWordIndexes.empty())
    {
        mSelectedWordIndexes.pop_back();
        bool anyWordsLeft = RefreshSelectedAndAvailableWords();

        // If erasing a word after an incorrect guess, we need to reset the message label to the default message.
        // Also change the state back to selecting words.
        if(anyWordsLeft && mState == State::Unsolved)
        {
            mState = State::SelectWords;
            mMessagesLabel->SetText("");
            AppendMessageText("LatinMsg");
            AppendMessageText("Latin2Msg");
            AppendMessageText("SelectMsg");
        }

        // The erase button should be active as long as there are still selected words.
        mEraseButton->GetButton()->SetCanInteract(!mSelectedWordIndexes.empty());
    }
}