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
            // Executing the callback can itself cause the callback to be set (if multiple dialogs in a row).
            // So, set member to null before executing it.
            auto callback = mDialogueFinishCallback;
			mDialogueFinishCallback = nullptr;
            callback();
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
		GKActor* actor = GEngine::Instance()->GetScene()->GetActorByNoun(mSpeaker);
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
		GKActor* actor = GEngine::Instance()->GetScene()->GetActorByNoun(mSpeaker);
		if(actor != nullptr)
		{
			actor->StartFidget(GKActor::FidgetType::Talk);
		}
	}
}

void DialogueManager::SetConversation(const std::string& conversation, std::function<void()> finishCallback)
{
    // Now in a conversation!
	mConversation = conversation;
    std::cout << "SetConversation " << conversation << std::endl;

    // Save callback.
    mConversationAnimFinishCallback = finishCallback;

    // See if there are any dialogue cameras associated with starting this conversation (isInitial = true).
    // If so, set that camera angle.
    GEngine::Instance()->GetScene()->SetCameraPositionForConversation(conversation, true);

    // Apply settings for this conversation.
    // Some actors may use different talk/listen GAS for particular conversations.
    // And some actors may need to play enter anims when starting a conversation.
    mConversationAnimWaitCount = 0;
    std::vector<const SceneConversation*> conversationSettings = GEngine::Instance()->GetScene()->GetSceneData()->GetConversationSettings(conversation);
    for(auto& settings : conversationSettings)
    {
        // If needed, set new GAS for actor.
        if(settings->talkGas != nullptr || settings->listenGas != nullptr)
        {
            GKActor* actor = GEngine::Instance()->GetScene()->GetActorByNoun(settings->actorName);
            if(actor != nullptr)
            {
                if(settings->talkGas != nullptr)
                {
                    actor->SetTalkFidget(settings->talkGas);

                    //TODO: Conversations often set both Talk & Idle fidgets. Looking at Gabe, it appears he starts playing his talk fidget automatically.
                    //TODO: But if both participants set fidgets, how do you know who should start in Talk vs. Listen mode? Maybe Ego is always Talk first?
                    actor->StartFidget(GKActor::FidgetType::Talk);
                }
                if(settings->listenGas != nullptr)
                {
                    actor->SetListenFidget(settings->listenGas);
                }
            }
        }

        // Play enter anim.
        if(settings->enterAnim != nullptr)
        {
            ++mConversationAnimWaitCount;
            GEngine::Instance()->GetScene()->GetAnimator()->Start(settings->enterAnim, [this]() {
                --mConversationAnimWaitCount;
                CheckConversationAnimFinishCallback();
            });
        }
    }

    // No waits? Do callback right away.
    CheckConversationAnimFinishCallback();
}

void DialogueManager::EndConversation(std::function<void()> finishCallback)
{
    // No conversation? No problem.
    if(mConversation.empty())
    {
        if(finishCallback != nullptr)
        {
            finishCallback();
        }
        return;
    }

    // Save callback.
    mConversationAnimFinishCallback = finishCallback;

    // See if there are any dialogue cameras associated with ending this conversation (isFinal = true).
    // If so, set that camera angle.
    GEngine::Instance()->GetScene()->SetCameraPositionForConversation(mConversation, false);

    // Play any exit anims for actors in this conversation.
    mConversationAnimWaitCount = 0;
    std::vector<const SceneConversation*> conversationSettings = GEngine::Instance()->GetScene()->GetSceneData()->GetConversationSettings(mConversation);
    for(auto& settings : conversationSettings)
    {
        //TODO: Do we need to revert any GAS changes from starting the conversation? Or are these naturally resolved in later conversations?

        // Play exit anim.
        if(settings->exitAnim != nullptr)
        {
            ++mConversationAnimWaitCount;
            GEngine::Instance()->GetScene()->GetAnimator()->Start(settings->exitAnim, [this]() {
                --mConversationAnimWaitCount;
                CheckConversationAnimFinishCallback();
            });
        }

        // Have the actor go back to their idle fidget.
        // We don't know all participants in a conversation - that data isn't stored in SIF or anything :P
        // But if we have a conversation setting for an actor, at least we know that.
        GKActor* actor = GEngine::Instance()->GetScene()->GetActorByNoun(settings->actorName);
        if(actor != nullptr)
        {
            actor->StartFidget(GKActor::FidgetType::Idle);
        }
    }

    // No waits? Do callback right away.
    CheckConversationAnimFinishCallback();

    // No longer in this conversation.
    std::cout << "EndConversation " << mConversation << std::endl;
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
	GEngine::Instance()->GetScene()->GetAnimator()->StartYak(yak, nullptr);
}

void DialogueManager::CheckConversationAnimFinishCallback()
{
    if(mConversationAnimWaitCount == 0 && mConversationAnimFinishCallback != nullptr)
    {
        auto callback = mConversationAnimFinishCallback;
        mConversationAnimFinishCallback = nullptr;
        callback();
    }
}