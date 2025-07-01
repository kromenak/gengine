//
// Clark Kromenaker
//
// The Anagram Parser is Sidney tool that allows the player to solve anagrams.
//
#pragma once
#include "Actor.h"

#include <functional>
#include <string>
#include <vector>

#include "AudioManager.h"

class SidneyButton;
class UILabel;
class UIScrollRect;

class SidneyAnagramParser : public Actor
{
public:
    SidneyAnagramParser(Actor* parent);

    void Show(const std::string& anagramText, const std::function<void()>& exitCallback);
    void Hide();

protected:
    void OnUpdate(float deltaTime) override;

private:
    enum class State
    {
        Idle,
        InitialScramble,
        SelectWords,
        FinalScramble,
        Solved,
        Unsolved
    };
    State mState = State::Idle;

    // A header label showing the text currently being parsed.
    UILabel* mParsingHeaderLabel = nullptr;

    // One label per letter of the anagram text.
    static const int kMaxAnagramLetters = 17;
    UILabel* mLetterLabels[kMaxAnagramLetters] = { 0 };

    // A label that shows feedback messages from the parser system.
    UILabel* mMessagesLabel = nullptr;

    // A scroll rect that contains the discovered anagram words.
    UIScrollRect* mAvailableWordsScrollRect = nullptr;

    // A list of words that are available to select from to solve the anagram.
    static const int kMaxAnagramWords = 161;
    UILabel* mAvailableWordLabels[kMaxAnagramWords] = { 0 };

    // The anagram erase and exit buttons. Need to be enabled/disabled at certain times.
    SidneyButton* mEraseButton = nullptr;
    SidneyButton* mExitButton = nullptr;

    // The anagram letters, with spaces stripped out.
    std::string mAllLetters;

    // The remaining letters.
    std::string mRemainingLetters;

    // A sound that plays when scrambling anagram letters.
    PlayingSoundHandle mScrambleSoundHandle;

    // A prefix for the words to populate in the anagram word list, how many to populate, and how many have been populated.
    std::string mLocWordsPrefix;
    int mWordsCount = 0;
    int mCurrentWordsIndex = 0;

    // When scrambling, a timer that controls how frequently the anagram letter labels rearrange.
    const float kScrambleInterval = 0.15f;
    float mScrambleTimer = 0.0f;

    // When scrambling, a timer that controls how frequently a "discovered" word is added to the available list.
    const float kScrambleAddWordInitialDelay = 4.0f;
    const float kScrambleAddWordInterval = 0.15f;
    float mScrambleAddWordTimer = 0.0f;

    // When the correct words are selected, there's a final scramble for a set duration before revealing the hidden message.
    const float kFinalScrambleDuration = 10.0f;
    float mFinalScrambleTimer = 0.0f;

    // Labels to display each selected word.
    static const int kMaxSelectedWords = 10;
    UILabel* mSelectedWordLabels[kMaxSelectedWords] = { 0 };

    // The indexes (in the available word labels array) of words that have been selected by the player.
    std::vector<int> mSelectedWordIndexes;

    // A label that displays the translated text as one string.
    UILabel* mTranslatedTextLabel = nullptr;

    // A callback executed if the "exit" button is pressed.
    std::function<void()> mExitCallback = nullptr;

    void AppendMessageText(const std::string& locKey);

    bool RefreshSelectedAndAvailableWords();
    void RefreshAvailableWordsScrollList();

    void StartScramble();
    void StopScramble();

    void OnAvailableWordSelected(int index);
    void OnEraseButtonPressed();
};