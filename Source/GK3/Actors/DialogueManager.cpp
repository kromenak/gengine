#include "DialogueManager.h"

#include <cctype>

#include "ActionManager.h"
#include "AssetManager.h"
#include "Animation.h"
#include "Animator.h"
#include "GameCamera.h"
#include "GKActor.h"
#include "SceneManager.h"

DialogueManager gDialogueManager;

void DialogueManager::StartDialogue(const std::string& licensePlate, int numLines, bool playFidgets, std::function<void()> finishCallback)
{
	// We need a valid license plate.
	if(licensePlate.empty()) { return; }
	//TODO: Can we assume/expect a certain length for license plates?
	
	// The last character is always the sequence number, but it can be 0-9 or A-Z.
    // Convert it to a normal integer.
	char sequenceChar = licensePlate.back();
	if(std::isdigit(sequenceChar)) // Number 0-9
	{
		mDialogueSequenceNumber = sequenceChar - '0';
	}
	else // Alpha character A-Z
	{
        mDialogueSequenceNumber = (sequenceChar - 'A');
        mDialogueSequenceNumber += 10;
	}
	
	// Save the license plate, but chop off the sequence number.
    mDialogueLicensePlate = licensePlate;
    mDialogueLicensePlate.pop_back();
	
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
    // Ignore setting speaker if already set to that person.
    // This is actually kind of important sometimes, to avoid playing fidgets when not intended.
    if(StringUtil::EqualsIgnoreCase(mSpeaker, noun))
    {
        return;
    }
    
	// If someone is no longer the speaker, have them transition to listening.
    bool isUnknownSpeaker = StringUtil::EqualsIgnoreCase(mSpeaker, "UNKNOWN");
	if(!mSpeaker.empty() && !isUnknownSpeaker && mDialogueUsesFidgets)
	{
		GKActor* actor = gSceneManager.GetScene()->GetActorByNoun(mSpeaker);
		if(actor != nullptr)
		{
			actor->StartFidget(GKActor::FidgetType::Listen);
		}
	}
	
	// Set new speaker.
	mSpeaker = noun;
	
	// Have the new speaker play talk animation.
	if(mDialogueUsesFidgets && !isUnknownSpeaker)
	{
		GKActor* actor = gSceneManager.GetScene()->GetActorByNoun(mSpeaker);
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
    //std::cout << "SetConversation " << conversation << std::endl;

    // Save callback.
    mConversationAnimFinishCallback = finishCallback;

    // See if there are any dialogue cameras associated with starting this conversation (isInitial = true).
    // If so, set that camera angle.
    if(GameCamera::AreCinematicsEnabled())
    {
        gSceneManager.GetScene()->SetCameraPositionForConversation(conversation, true);
    }

    // Clear any previously saved fidgets.
    mSavedTalkFidgets.clear();
    mSavedListenFidgets.clear();

    // Apply settings for this conversation.
    // Some actors may use different talk/listen GAS for particular conversations.
    // And some actors may need to play enter anims when starting a conversation.
    mConversationAnimWaitCount = 0;
    std::vector<const SceneConversation*> conversationSettings = gSceneManager.GetScene()->GetSceneData()->GetConversationSettings(conversation);
    for(auto& settings : conversationSettings)
    {
        // If needed, set new GAS for actor.
        if(settings->talkGas != nullptr || settings->listenGas != nullptr)
        {
            GKActor* actor = gSceneManager.GetScene()->GetActorByNoun(settings->actorName);
            if(actor != nullptr)
            {
                if(settings->talkGas != nullptr)
                {
                    mSavedTalkFidgets.emplace_back(actor, actor->GetTalkFidget());
                    actor->SetTalkFidget(settings->talkGas);
                }
                if(settings->listenGas != nullptr)
                {
                    mSavedListenFidgets.emplace_back(actor, actor->GetListenFidget());
                    actor->SetListenFidget(settings->listenGas);

                    // Start with the listen fidget.
                    actor->StartFidget(GKActor::FidgetType::Listen);
                }

                // Use the talk fidget by default if there were no listen fidget defined for some reason.
                if(settings->talkGas != nullptr && settings->listenGas == nullptr)
                {
                    actor->StartFidget(GKActor::FidgetType::Talk);
                }
            }
        }

        // Play enter anim.
        if(settings->enterAnim != nullptr)
        {
            ++mConversationAnimWaitCount;
            gSceneManager.GetScene()->GetAnimator()->Start(settings->enterAnim, [this]() {
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
    if(GameCamera::AreCinematicsEnabled())
    {
        gSceneManager.GetScene()->SetCameraPositionForConversation(mConversation, false);
    }

    // Revert any fidgets that were set when entering the conversation.
    for(auto& pair : mSavedTalkFidgets)
    {
        pair.first->SetTalkFidget(pair.second);
    }
    for(auto& pair : mSavedListenFidgets)
    {
        pair.first->SetListenFidget(pair.second);
    }
    mSavedTalkFidgets.clear();
    mSavedListenFidgets.clear();

    // Play any exit anims for actors in this conversation.
    mConversationAnimWaitCount = 0;
    std::vector<const SceneConversation*> conversationSettings = gSceneManager.GetScene()->GetSceneData()->GetConversationSettings(mConversation);
    for(auto& settings : conversationSettings)
    {
        // Play exit anim.
        if(settings->exitAnim != nullptr)
        {
            ++mConversationAnimWaitCount;
            gSceneManager.GetScene()->GetAnimator()->Start(settings->exitAnim, [this]() {
                --mConversationAnimWaitCount;
                CheckConversationAnimFinishCallback();
            });
        }

        // Have the actor go back to their idle fidget.
        // We don't know all participants in a conversation - that data isn't stored in SIF or anything :P
        // But if we have a conversation setting for an actor, at least we know that.
        GKActor* actor = gSceneManager.GetScene()->GetActorByNoun(settings->actorName);
        if(actor != nullptr)
        {
            actor->StartFidget(GKActor::FidgetType::Idle);
        }
    }

    // No waits? Do callback right away.
    CheckConversationAnimFinishCallback();

    // No longer in this conversation.
    //std::cout << "EndConversation " << mConversation << std::endl;
	mConversation.clear();
}

void DialogueManager::PlayNextDialogueLine()
{
	// Playing a line, so decrement remaining lines.
	mRemainingDialogueLines--;

    // If we're skipping an action right now, it seems safe (?) to just trigger the dialogue cue and skip loading the YAK entirely.
    if(gActionManager.IsSkippingCurrentAction())
    {
        TriggerDialogueCue();
        return;
    }

    // Construct YAK name from stored plate/sequence number.
    std::string yakName = "E" + mDialogueLicensePlate;
    if(mDialogueSequenceNumber < 10)
    {
        yakName += ('0' + static_cast<char>(mDialogueSequenceNumber));
    }
    else
    {
        yakName += ('A' + static_cast<char>(mDialogueSequenceNumber - 10));
    }

    // Increment sequence number.
    mDialogueSequenceNumber++;

	// Load the YAK! If we can't find it for some reason, output an error and move on right away.
	Animation* yak = gAssetManager.LoadYak(yakName, AssetScope::Scene);
	if(yak == nullptr)
	{
		std::cout << "Could not find YAK called " << yakName << ". Skipping to next dialogue line." << std::endl;
		TriggerDialogueCue();
		return;
	}

    // Play the YAK.
    // To trigger the next line of dialogue, YAKs contain a DIALOGUECUE, which causes "TriggerDialogueCue" to be called.
    AnimParams yakAnimParams;
    yakAnimParams.animation = yak;
    yakAnimParams.isYak = true;
    gSceneManager.GetScene()->GetAnimator()->Start(yakAnimParams);
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