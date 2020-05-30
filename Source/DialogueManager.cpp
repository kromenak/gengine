//
// DialogueManager.cpp
//
// Clark Kromenaker
//
#include "DialogueManager.h"

#include <cctype>

#include "ActionManager.h"
#include "Animation.h"
#include "Animator.h"
#include "GEngine.h"
#include "GKActor.h"
#include "Scene.h"
#include "Services.h"

TYPE_DEF_BASE(DialogueManager);

void DialogueManager::StartDialogue(const std::string& licensePlate, int numLines, bool playFidgets, std::function<void()> finishCallback)
{
	// We need a valid license plate.
	if(licensePlate.empty()) { return; }
	//TODO: Can we assume/expect a certain length for license plates?
	
	// Determine the sequence number of the license plate.
	// The last character is always the sequence number, but it can be 1-9 or A-Z.
	char sequenceChar = licensePlate.back();
	if(std::isdigit(sequenceChar)) // Number 1-9
	{
		mDialogueSequenceNumber = sequenceChar - '0';
	}
	else // Alpha character A-Z
	{
		mDialogueSequenceNumber = sequenceChar - 'A';
	}
	
	// Save the license plate, but chop off the sequence number.
	mDialogueLicensePlate = licensePlate.substr(0, licensePlate.size() - 1);
	
	// Save remaining lines and finish callback.
	mRemainingDialogueLines = numLines;
	mDialogueUsesFidgets = playFidgets;
	mDialogueFinishCallback = finishCallback;
	
	// Play first line of dialogue.
	PlayNextDialogueLine();
}

void DialogueManager::ContinueDialogue(int numLines, bool playFidgets, std::function<void()> finishCallback)
{
	// This assumes that we've already previously specified a plate/sequence and we just want to continue the sequence.
	mRemainingDialogueLines = numLines;
	mDialogueUsesFidgets = playFidgets;
	mDialogueFinishCallback = finishCallback;
	
	// Play next line.
	PlayNextDialogueLine();
}

void DialogueManager::TriggerDialogueCue()
{
	// If we've done all the lines of dialogue we're interested in...
	if(mRemainingDialogueLines <= 0)
	{
		// Call finish callback.
		if(mDialogueFinishCallback != nullptr)
		{
			mDialogueFinishCallback();
			mDialogueFinishCallback = nullptr;
		}
		return;
	}
	
	// We still have dialogue to execute!
	PlayNextDialogueLine();
}

void DialogueManager::SetSpeaker(const std::string& noun)
{
	// If someone is no longer the speaker, have them transition to listening.
	if(!mSpeaker.empty() && mDialogueUsesFidgets)
	{
		GKActor* actor = GEngine::inst->GetScene()->GetActorByNoun(mSpeaker);
		if(actor != nullptr)
		{
			actor->StartFidget(GKActor::FidgetType::Listen);
		}
	}
	
	// Set new speaker.
	mSpeaker = noun;
	
	// Have the new speaker play talk animation.
	if(mDialogueUsesFidgets)
	{
		GKActor* actor = GEngine::inst->GetScene()->GetActorByNoun(mSpeaker);
		if(actor != nullptr)
		{
			actor->StartFidget(GKActor::FidgetType::Talk);
		}
	}
}

void DialogueManager::SetConversation(const std::string& conversation)
{
	mConversation = conversation;
	//TODO: Set camera - should get dialogue camera(s) with matching dialogueName, set to "initial" one
	//TODO: Gather LISTENERS from SIF and play appropriate anims, set fidgets, etc.
	std::cout << "SetConversation " << conversation << std::endl;
}

void DialogueManager::EndConversation()
{
	//TODO: Set camera - get dialogue camera(s) with matching dialogueName, set to "final" one
	//TODO: Based on listeners involved, play any exit anims, clear fidgets, etc.
	mConversation.clear();
}

void DialogueManager::PlayNextDialogueLine()
{
	// Construct YAK name from stored plate/sequence number.
	std::string yakName = "E" + mDialogueLicensePlate;
	if(mDialogueSequenceNumber < 10)
	{
		yakName += ('0' + mDialogueSequenceNumber);
	}
	else
	{
		yakName += ('A' + mDialogueSequenceNumber);
	}
	
	// Increment sequence number.
	mDialogueSequenceNumber++;
	
	// Playing a line, so decrement remaining lines.
	mRemainingDialogueLines--;
	
	// Load and execute the YAK!
	// To trigger the next line of dialogue, YAKs contain a DIALOGUECUE, which causes "TriggerDialogueCue" to be called (below).
	Animation* yak = Services::GetAssets()->LoadYak(yakName);
	if(yak == nullptr)
	{
		std::cout << "Could not find YAK called " << yakName << ". Skipping to next dialogue line." << std::endl;
		TriggerDialogueCue();
		return;
	}
	std::cout << "Playing YAK " << yak->GetName() << std::endl;
	GEngine::inst->GetScene()->GetAnimator()->Start(yak, false, false, nullptr);
}
