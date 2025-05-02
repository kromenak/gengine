#include "SidneyAnalyze.h"

#include "ActionManager.h"
#include "Actor.h"
#include "AssetManager.h"
#include "GameProgress.h"
#include "Random.h"
#include "SidneyButton.h"
#include "SidneyFiles.h"
#include "SidneyTranslate.h"
#include "SidneyUtil.h"
#include "UIButton.h"
#include "UICanvas.h"
#include "UIImage.h"
#include "UINineSlice.h"
#include "UIScrollRect.h"
#include "UIUtil.h"

void SidneyAnalyze::AnalyzeText_Init()
{
    // Create window for analyzing text.
    {
        mAnalyzeTextWindow = new Actor(TransformType::RectTransform);
        mAnalyzeTextWindow->GetTransform()->SetParent(mRoot->GetTransform());

        UINineSlice* border = mAnalyzeTextWindow->AddComponent<UINineSlice>(SidneyUtil::GetGrayBoxParams(SidneyUtil::TransBgColor));
        border->GetRectTransform()->SetSizeDelta(300.0f, 104.0f);

        // Add divider line for box header.
        UIImage* boxHeaderDividerImage = UIUtil::NewUIActorWithWidget<UIImage>(mAnalyzeTextWindow);
        boxHeaderDividerImage->SetTexture(gAssetManager.LoadTexture("S_BOX_TOP.BMP"), true);
        boxHeaderDividerImage->GetRectTransform()->SetAnchor(AnchorPreset::TopStretch);
        boxHeaderDividerImage->GetRectTransform()->SetAnchoredPosition(0.0f, -17.0f);
        boxHeaderDividerImage->GetRectTransform()->SetSizeDeltaX(0.0f);

        // Add header label.
        mAnalyzeTextFileNameLabel = UIUtil::NewUIActorWithWidget<UILabel>(mAnalyzeTextWindow);
        mAnalyzeTextFileNameLabel->SetFont(gAssetManager.LoadFont("SID_TEXT_14.FON"));
        mAnalyzeTextFileNameLabel->SetHorizonalAlignment(HorizontalAlignment::Right);
        mAnalyzeTextFileNameLabel->SetVerticalAlignment(VerticalAlignment::Top);
        mAnalyzeTextFileNameLabel->SetMasked(true);

        mAnalyzeTextFileNameLabel->GetRectTransform()->SetAnchor(AnchorPreset::TopRight);
        mAnalyzeTextFileNameLabel->GetRectTransform()->SetAnchoredPosition(-8.0f, -3.0f);
        mAnalyzeTextFileNameLabel->GetRectTransform()->SetSizeDeltaY(14.0f);

        // Add text label.
        mAnalyzeTextLabel = UIUtil::NewUIActorWithWidget<UILabel>(mAnalyzeTextWindow);
        mAnalyzeTextLabel->SetFont(gAssetManager.LoadFont("SID_TEXT_14.FON"));
        mAnalyzeTextLabel->SetHorizonalAlignment(HorizontalAlignment::Center);
        mAnalyzeTextLabel->SetVerticalAlignment(VerticalAlignment::Center);
        mAnalyzeTextLabel->GetRectTransform()->SetAnchor(AnchorPreset::TopStretch);
        mAnalyzeTextLabel->GetRectTransform()->SetAnchoredPosition(0.0f, -17.0f);
        mAnalyzeTextLabel->GetRectTransform()->SetSizeDeltaY(88.0f);
    }

    // Create window for anagram parser.
    {
        // Create overall window with border.
        UINineSlice* window = UIUtil::NewUIActorWithWidget<UINineSlice>(mRoot, SidneyUtil::GetGrayBoxParams(SidneyUtil::TransBgColor));
        window->GetRectTransform()->SetSizeDelta(600.0f, 319.0f);
        mAnagramParserWindow = window->GetOwner();

        // Lay out divider lines for the various areas of the anagram parser window.
        {
            UIImage* dividerImage = UIUtil::NewUIActorWithWidget<UIImage>(window->GetOwner());
            dividerImage->SetTexture(gAssetManager.LoadTexture("S_BOX_TOP.BMP"), true);
            dividerImage->GetRectTransform()->SetAnchor(AnchorPreset::TopStretch);
            dividerImage->GetRectTransform()->SetAnchoredPosition(0.0f, -61.0f);
            dividerImage->GetRectTransform()->SetSizeDeltaX(0.0f);

            dividerImage = UIUtil::NewUIActorWithWidget<UIImage>(window->GetOwner());
            dividerImage->SetTexture(gAssetManager.LoadTexture("S_BOX_TOP.BMP"), true);
            dividerImage->GetRectTransform()->SetAnchor(AnchorPreset::TopStretch);
            dividerImage->GetRectTransform()->SetAnchoredPosition(0.0f, -231.0f);
            dividerImage->GetRectTransform()->SetSizeDeltaX(0.0f);

            dividerImage = UIUtil::NewUIActorWithWidget<UIImage>(window->GetOwner());
            dividerImage->SetTexture(gAssetManager.LoadTexture("S_BOX_TOP.BMP"), true);
            dividerImage->GetRectTransform()->SetAnchor(AnchorPreset::TopStretch);
            dividerImage->GetRectTransform()->SetAnchoredPosition(0.0f, -291.0f);
            dividerImage->GetRectTransform()->SetSizeDeltaX(0.0f);

            dividerImage = UIUtil::NewUIActorWithWidget<UIImage>(window->GetOwner());
            dividerImage->SetTexture(gAssetManager.LoadTexture("S_BOX_SIDE.BMP"), true);
            dividerImage->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
            dividerImage->GetRectTransform()->SetAnchoredPosition(183.0f, -61.0f);
            dividerImage->GetRectTransform()->SetSizeDelta(2.0f, 171.0f);
        }

        // Create labels in the header area.
        {
            UILabel* titleLabel = UIUtil::NewUIActorWithWidget<UILabel>(window->GetOwner());
            titleLabel->SetFont(gAssetManager.LoadFont("SID_TEXT_14.FON"));
            titleLabel->SetText(SidneyUtil::GetAnalyzeLocalizer().GetText("AnagramTitle"));
            titleLabel->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
            titleLabel->GetRectTransform()->SetAnchoredPosition(15.0f, -9.5f);
            titleLabel->FitRectTransformToText();

            mAnagramParsingTextLabel = UIUtil::NewUIActorWithWidget<UILabel>(window->GetOwner());
            mAnagramParsingTextLabel->SetFont(gAssetManager.LoadFont("SID_TEXT_14.FON"));
            mAnagramParsingTextLabel->SetText(SidneyUtil::GetAnalyzeLocalizer().GetText("Parsing"));
            mAnagramParsingTextLabel->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
            mAnagramParsingTextLabel->GetRectTransform()->SetAnchoredPosition(161.0f, -9.5f);
            mAnagramParsingTextLabel->FitRectTransformToText();

            const float kLetterX = 88.0f;
            const float kLetterSpacing = 20.0f;
            for(int i = 0; i < 17; ++i)
            {
                mAnagramLetterLabels[i] = UIUtil::NewUIActorWithWidget<UILabel>(window->GetOwner());
                mAnagramLetterLabels[i]->SetFont(gAssetManager.LoadFont("SID_TEXT_14.FON"));
                mAnagramLetterLabels[i]->SetText("A");
                mAnagramLetterLabels[i]->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
                mAnagramLetterLabels[i]->GetRectTransform()->SetAnchoredPosition(kLetterX + i * kLetterSpacing, -34.5f);
                mAnagramLetterLabels[i]->FitRectTransformToText();
            }
        }

        // Create scrollable "select" area to take letters from.
        {
            // Add scroll area
            {
                UIImage* background = UIUtil::NewUIActorWithWidget<UIImage>(window->GetOwner());
                background->SetColor(Color32(0, 0, 0, 255));
                background->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
                background->GetRectTransform()->SetAnchoredPosition(1.0f, -62.0f);
                background->GetRectTransform()->SetSizeDelta(182.0f, 169.0f);

                UICanvas* scrollCanvas = UIUtil::NewUIActorWithCanvas(window->GetOwner(), -1);
                scrollCanvas->SetMasked(true);
                scrollCanvas->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
                scrollCanvas->GetRectTransform()->SetAnchoredPosition(1.0f, -62.0f);
                scrollCanvas->GetRectTransform()->SetSizeDelta(182.0f, 169.0f);

                UIScrollRect* scrollRect = new UIScrollRect(scrollCanvas->GetOwner());
                scrollRect->GetRectTransform()->SetAnchor(AnchorPreset::CenterStretch);
                scrollRect->GetRectTransform()->SetSizeDelta(0.0f, 0.0f);
                scrollRect->SetScrollbarWidth(5.0f);
                mAnagramWordsScrollRect = scrollRect;

                const float kSelectLabelX = 14.0f;
                const float kSelectLabelStartY = -25.5f;
                const float kSelectLabelSpacing = 15.0f;
                for(int i = 0; i < kMaxAnagramWords; ++i)
                {
                    UILabel* label = UIUtil::NewUIActorWithWidget<UILabel>(scrollRect);
                    label->SetFont(gAssetManager.LoadFont("SID_TEXT_14.FON"));
                    label->SetText(SidneyUtil::GetAnalyzeLocalizer().GetText("Word" + std::to_string(i + 1)));
                    label->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
                    label->GetRectTransform()->SetAnchoredPosition(kSelectLabelX, kSelectLabelStartY - (i * kSelectLabelSpacing));
                    label->FitRectTransformToText();
                    mAnagramWordLabels[i] = label;

                    UIButton* button = label->GetOwner()->AddComponent<UIButton>();
                    button->SetPressCallback([i](UIButton* button){
                        printf("%s\n", SidneyUtil::GetAnalyzeLocalizer().GetText("Word" + std::to_string(i + 1)).c_str());
                    });
                }

                // Add header box
                UINineSlice* box = UIUtil::NewUIActorWithWidget<UINineSlice>(scrollCanvas->GetOwner(), SidneyUtil::GetGrayBoxParams(Color32::Black));
                box->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
                box->GetRectTransform()->SetAnchoredPosition(-1.0f, 1.0f);
                box->GetRectTransform()->SetSizeDelta(60.0f, 14.0f);

                // Add header label.
                UILabel* label = UIUtil::NewUIActorWithWidget<UILabel>(box->GetOwner());
                label->SetFont(gAssetManager.LoadFont("SID_PDN_10_UL.FON"));
                label->SetText(SidneyUtil::GetAnalyzeLocalizer().GetText("SelectText"));
                label->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
                label->GetRectTransform()->SetAnchoredPosition(8.0f, -2.5f);
                label->FitRectTransformToText();
            }
        }

        // Create message area.
        {
            mAnagramMessagesLabel = UIUtil::NewUIActorWithWidget<UILabel>(window->GetOwner());
            mAnagramMessagesLabel->SetFont(gAssetManager.LoadFont("SID_TEXT_14.FON"));
            mAnagramMessagesLabel->SetText(SidneyUtil::GetAnalyzeLocalizer().GetText("LatinMsg") + "\n" + SidneyUtil::GetAnalyzeLocalizer().GetText("Latin2Msg"));
            mAnagramMessagesLabel->SetVerticalAlignment(VerticalAlignment::Top);
            mAnagramMessagesLabel->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
            mAnagramMessagesLabel->GetRectTransform()->SetAnchoredPosition(193.0f, -68.5f);
            mAnagramMessagesLabel->GetRectTransform()->SetSizeDelta(405.0f, 160.0f);
        }

        // Create "phrase building area."
        {
            // Add header box.
            UINineSlice* box = UIUtil::NewUIActorWithWidget<UINineSlice>(window->GetOwner(), SidneyUtil::GetGrayBoxParams(Color32::Black));
            box->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
            box->GetRectTransform()->SetAnchoredPosition(0.0f, -231.0f);
            box->GetRectTransform()->SetSizeDelta(160.0f, 14.0f);

            // Add header label.
            UILabel* label = UIUtil::NewUIActorWithWidget<UILabel>(box->GetOwner());
            label->SetFont(gAssetManager.LoadFont("SID_PDN_10_UL.FON"));
            label->SetText(SidneyUtil::GetAnalyzeLocalizer().GetText("PhraseText"));
            label->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
            label->GetRectTransform()->SetAnchoredPosition(8.0f, -2.5f);
            label->FitRectTransformToText();
        }


        // Create footer.
        {
            SidneyButton* eraseButton = SidneyUtil::CreateSmallButton(window->GetOwner());
            eraseButton->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
            eraseButton->GetRectTransform()->SetAnchoredPosition(10.0f, -299.0f);
            eraseButton->SetWidth(60.0f);
            eraseButton->SetText(SidneyUtil::GetAnalyzeLocalizer().GetText("EraseButton"));
            eraseButton->SetPressCallback([this](){
                printf("Erase\n");
            });
            eraseButton->GetButton()->SetCanInteract(false);

            SidneyButton* exitButton = SidneyUtil::CreateSmallButton(window->GetOwner());
            exitButton->GetRectTransform()->SetAnchor(AnchorPreset::TopLeft);
            exitButton->GetRectTransform()->SetAnchoredPosition(83.0f, -299.0f);
            exitButton->SetWidth(60.0f);
            exitButton->SetText(SidneyUtil::GetAnalyzeLocalizer().GetText("ExitButton"));
            exitButton->SetPressCallback([this](){
                mAnagramParserWindow->SetActive(false);
            });
        }

        // Hide the anagram parser to start.
        mAnagramParserWindow->SetActive(false);
    }
}

void SidneyAnalyze::AnalyzeText_EnterState()
{
    // Show text analyze window.
    mAnalyzeTextWindow->SetActive(true);

    // Populate the file name label.
    mAnalyzeTextFileNameLabel->SetText(mSidneyFiles->GetFile(mAnalyzeFileId)->GetDisplayName());

    // "Text" dropdown is available when analyzing text - as you might expect.
    mMenuBar.SetDropdownEnabled(kTextDropdownIdx, true);
    mMenuBar.SetDropdownEnabled(kGraphicDropdownIdx, false);
    mMenuBar.SetDropdownEnabled(kMapDropdownIdx, false);

    // Populate the text field, based on file.
    if(mAnalyzeFileId == SidneyFileIds::kArcadiaText)
    {
        // Set text label. If figured out the "SUM" word to add, the text is slightly different.
        if(gGameProgress.GetFlag("ArcadiaTranslated"))
        {
            mAnalyzeTextLabel->SetText(SidneyUtil::GetAnalyzeLocalizer().GetText("ArcadiaText"));
        }
        else
        {
            mAnalyzeTextLabel->SetText(SidneyUtil::GetAnalyzeLocalizer().GetText("ArcadiaText2"));
        }

        // Populate enabled menu items.
        mMenuBar.SetDropdownChoiceEnabled(kTextDropdownIdx, kTextDropdown_ExtractAnomaliesIdx, false);
        mMenuBar.SetDropdownChoiceEnabled(kTextDropdownIdx, kTextDropdown_TranslateIdx, true);
        mMenuBar.SetDropdownChoiceEnabled(kTextDropdownIdx, kTextDropdown_AnagramParserIdx, true);
        mMenuBar.SetDropdownChoiceEnabled(kTextDropdownIdx, kTextDropdown_AnalyzeTextIdx, false);
    }
}

void SidneyAnalyze::AnalyzeText_Update(float deltaTime)
{
    // Only need to update the anagram parser window.
    if(!mAnagramParserWindow->IsActive()) { return; }

    // Decrement timer until next scramble.
    if(mAnagramScrambleTimer > 0.0f)
    {
        mAnagramScrambleTimer -= deltaTime;
        if(mAnagramScrambleTimer <= 0.0f)
        {
            // Do the scramble!
            std::string lettersRemaining = mAnagramLetters;
            for(int i = 0; i < kMaxAnagramLetters; ++i)
            {
                if(i >= mAnagramLetters.length()) { break; }

                int randomIndex = Random::RangeSize(0, lettersRemaining.length());
                mAnagramLetterLabels[i]->SetText(std::string(1, lettersRemaining[randomIndex]));
                lettersRemaining.erase(randomIndex, 1);
            }

            // Scramble again after X seconds have passed.
            mAnagramScrambleTimer = kAnagramScrambleIntervalSeconds;
        }
    }

    // Decrement timer until add another discovered word.
    if(mAddAnagramWordTimer > 0.0f)
    {
        mAddAnagramWordTimer -= deltaTime;
        if(mAddAnagramWordTimer <= 0.0f)
        {
            mAnagramWordLabels[mAnagramWordsIndex]->GetOwner()->SetActive(true);
            mAnagramWordLabels[mAnagramWordsIndex]->SetText(SidneyUtil::GetAnalyzeLocalizer().GetText(mAnagramWordsPrefix + std::to_string(mAnagramWordsIndex + 1)));
            //printf("%d - %s\n", mAnagramWordsIndex, mAnagramWordLabels[mAnagramWordsIndex]->GetText().c_str());
            ++mAnagramWordsIndex;

            if(mAnagramWordsIndex < mAnagramWordsCount)
            {
                mAddAnagramWordTimer = kAddAnagramWordIntervalSeconds;
            }
            else
            {
                // Stop the scramble SFX.
                gAudioManager.Stop(mAnagramScrambleSoundHandle);

                // Stop the word scramble effect.
                for(int i = 0; i < kMaxAnagramLetters; ++i)
                {
                    mAnagramLetterLabels[i]->SetText(std::string(1, mAnagramLetters[i]));
                }
                mAnagramScrambleTimer = 0.0f;

                // Show message indicating that all words have been found and now you have to choose them.
                std::string messagesText = mAnagramMessagesLabel->GetText();
                messagesText += "\n\n" + SidneyUtil::GetAnalyzeLocalizer().GetText("SelectMsg");
                mAnagramMessagesLabel->SetText(messagesText);

                // Should be interactive again.
                gActionManager.FinishManualAction();
            }
        }
    }
}

void SidneyAnalyze::AnalyzeText_OnAnalyzeButtonPressed()
{
    if(mAnalyzeFileId == SidneyFileIds::kArcadiaText)
    {
        ShowAnalyzeMessage("ArcadiaAnalysis", Vector2(), HorizontalAlignment::Left);
    }
}

void SidneyAnalyze::AnalyzeText_OnTranslateButtonPressed()
{
    Hide();
    mSidneyTranslate->Show(mAnalyzeFileId);
}

void SidneyAnalyze::AnalyzeText_OnAnagramParserPressed()
{
    // The anagram parser is used only in one instance, with Arcadia text while in the Orphiuchus step of LSR.
    if(mAnalyzeFileId == SidneyFileIds::kArcadiaText && SidneyUtil::GetCurrentLSRStep() == 10)
    {
        if(!mAnagramParserWindow->IsActive())
        {
            // Hide the analyze text window and show the parser window instead.
            mAnalyzeTextWindow->SetActive(false);
            mAnagramParserWindow->SetActive(true);

            // Make sure all select words are turned off to start.
            for(int i = 0; i < kMaxAnagramWords; ++i)
            {
                mAnagramWordLabels[i]->GetOwner()->SetActive(false);
            }

            // Figure out which text will be fed into the anagram parser.
            // Text differs based on whether you've figured out the missing "SUM" word.
            std::string anagramText = gGameProgress.GetFlag("ArcadiaTranslated") ?
                SidneyUtil::GetAnalyzeLocalizer().GetText("ArcadiaText") :
                SidneyUtil::GetAnalyzeLocalizer().GetText("ArcadiaText2");

            // Populate the "Parsing: " text field.
            mAnagramParsingTextLabel->SetText(SidneyUtil::GetAnalyzeLocalizer().GetText("Parsing") + " " + anagramText);

            // The letters is the text without spaces, all uppercase.
            mAnagramLetters = anagramText;
            StringUtil::RemoveAll(mAnagramLetters, ' ');
            StringUtil::ToUpper(mAnagramLetters);

            // Populate individual anagram parser letters.
            for(int i = 0; i < kMaxAnagramLetters; ++i)
            {
                if(i < mAnagramLetters.length())
                {
                    mAnagramLetterLabels[i]->SetEnabled(true);
                    mAnagramLetterLabels[i]->SetText(std::string(1, mAnagramLetters[i]));
                }
                else
                {
                    mAnagramLetterLabels[i]->SetEnabled(false);
                }
            }

            // Clear all text from the message area.
            mAnagramMessagesLabel->SetText("");

            // Pause for dramatic effect; I guess Sidney is thinking about it...
            gActionManager.ExecuteSheepAction("wait SetTimerSeconds(2)", [this](const Action* action){

                // Show message that essentially says "searching for latin words in this phrase."
                mAnagramMessagesLabel->SetText(SidneyUtil::GetAnalyzeLocalizer().GetText("LatinMsg") + "\n" +
                                               SidneyUtil::GetAnalyzeLocalizer().GetText("Latin2Msg"));

                // Play a "we're scrambling the letters" sound.
                PlayAudioParams audioParams;
                audioParams.audio = gAssetManager.LoadAudio("SIDANAGRAMSCRAMBLE.WAV", AssetScope::Scene);
                audioParams.audioType = AudioType::SFX;
                audioParams.loopCount = -1;
                mAnagramScrambleSoundHandle = gAudioManager.Play(audioParams);

                // Start the scramble pretty much right away.
                mAnagramScrambleTimer = 0.001f;

                // The game is non-interactive until the scramble completes.
                gActionManager.StartManualAction();

                // The number of anagram words detected differs based on whether "SUM" has been added.
                if(gGameProgress.GetFlag("ArcadiaTranslated"))
                {
                    mAnagramWordsPrefix = "Word";
                    mAnagramWordsCount = 161;
                }
                else
                {
                    mAnagramWordsPrefix = "WordB";
                    mAnagramWordsCount = 36;
                }

                // There's an initial delay before words start being found/added from the parser.
                mAddAnagramWordTimer = kAddAnagramWordInitialDelay;
            });
        }
    }
    else
    {
        // In this case, Grace says "I don't think I need to use that on this text" - or something along those lines.
        gActionManager.ExecuteSheepAction("wait StartDialogue(\"02O3H2Z951\", 1)");
    }
}