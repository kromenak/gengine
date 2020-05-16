//
// DialogueManager.h
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

#include "Type.h"

class DialogueManager
{
	TYPE_DECL_BASE();
public:
	void StartDialogue(const std::string& licensePlate, int numLines, bool playFidgets, std::function<void()> finishCallback);
	void ContinueDialogue(int numLines, bool playFidgets, std::function<void()> finishCallback);
	void TriggerDialogueCue();
	
	void SetSpeaker(const std::string& noun);
	
	void SetConversation(const std::string& conversationName);
	void EndConversation();
	
	void StartYak(const std::string& yakAnimName, std::function<void()> finishCallback);
	
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
	bool mDialogueUsesFidgets = true;
	
	// Callback to call when dialogue finishes.
	std::function<void()> mDialogueFinishCallback = nullptr;
	
	// The current speaker. Whoever is speaking will play a "talk" animation.
	// When someone else becomes the speaker, they'll play their "listen" animation.
	std::string mSpeaker;
	
	void PlayNextDialogueLine();
};
