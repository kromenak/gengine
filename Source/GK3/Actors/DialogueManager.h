//
// Clark Kromenaker
//
// Handles complexities of executing dialogue and conversations.
// Dialogue: a sequence of one or more "VO" animations (.YAK extension) containing audio, captions, and lip-sync parameters
// Conversation: a mode that is entered when you talk to a character - shows action bar with "topics" instead of "verbs."
//
#pragma once
#include <functional>
#include <string>
#include <vector>
#include <utility>

#include "Type.h"

class GKActor;
class GAS;

class DialogueManager
{
public:
	void StartDialogue(const std::string& licensePlate, int numLines, bool playFidgets, std::function<void()> finishCallback);
	void ContinueDialogue(int numLines, bool playFidgets, std::function<void()> finishCallback);
	void TriggerDialogueCue();
	
	void SetSpeaker(const std::string& noun);
    const std::string& GetSpeaker() const { return mSpeaker; }
	
	void SetConversation(const std::string& conversation, std::function<void()> finishCallback);
	void EndConversation(const std::function<void()> finishCallback);
    bool InConversation() const { return !mConversation.empty(); }
    
private:
	// A "license plate" is an identifier for a "YAK" or "VO WAV" file. They look like this: 0CAEI1IQ71
	// YAKs are localized; if the VO file is 0CAEI1IQ71, the YAK is E0CAEI1IQ71.YAK (the "E" indicates "English").
	// The last number indicates sequence. 0CAEI1IQ71 is first in sequence, 0CAEI1IQ72 is second, and so on.
	// If more than 9 exist in the sequence, letters A, B, C, D, etc are used.
	
	// The license plate that is currently playing. Empty if none.
	std::string mDialogueLicensePlate;
	
	// The "sequence number" of the dialogue license plate.
	// This is the last digit from the license plate. Goes 1-9, then uses letters A-Z.
	int mDialogueSequenceNumber = 0;
	
	// Number of lines left to play in current dialogue sequence.
	int mRemainingDialogueLines = 0;
	
	// Does the current dialogue use fidgets?
	// If not, we don't execute talk/listen fidgets on actors.
	bool mDialogueUsesFidgets = false;
	
	// Callback to call when dialogue finishes.
	std::function<void()> mDialogueFinishCallback = nullptr;
	
	// The current speaker. Whoever is speaking will play a "talk" animation.
	// When someone else becomes the speaker, they'll play their "listen" animation.
	std::string mSpeaker;
	
	// The current conversation. While a conversation is active, certain
	// cameras may be used and the topic chooser appears after dialogue.
	std::string mConversation;

    // Talk/listen fidgets to revert to when ending a conversation.
    std::vector<std::pair<GKActor*, GAS*>> mSavedTalkFidgets;
    std::vector<std::pair<GKActor*, GAS*>> mSavedListenFidgets;

    // When starting/ending a conversation, we may need to wait on one or more enter/exit anims.
    // Use this to keep track of how many anims are still outstanding.
    int mConversationAnimWaitCount = 0;

    // A callback to call once a conversation has been fully entered or exited.
    std::function<void()> mConversationAnimFinishCallback = nullptr;
	
	void PlayNextDialogueLine();

    void CheckConversationAnimFinishCallback();
};

extern DialogueManager gDialogueManager;