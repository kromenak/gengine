#include "DialogueManager.h"

#include <cctype>

#include "ActionManager.h"
#include "AssetManager.h"
#include "Animation.h"
#include "Animator.h"
#include "GameCamera.h"
#include "GKActor.h"
#include "Localizer.h"
#include "SceneManager.h"

DialogueManager gDialogueManager;

void DialogueManager::StartDialogue(const std::string& licensePlate, int numLines, bool playFidgets, const std::function<void()>& finishCallback)
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

    // Save remaining lines.
    mRemainingDialogueLines = numLines;

    // Save whether this dialogue plays fidgets.
    mDialogueUsesFidgets = playFidgets;

    // Add dialogue finish callback.
    mDialogueFinishCallbacks.push_back(finishCallback);

    // Play first line of dialogue.
    PlayNextDialogueLine();
}

void DialogueManager::ContinueDialogue(int numLines, bool playFidgets, const std::function<void()>& finishCallback)
{
    // This assumes that we've already previously specified a plate/sequence and we just want to continue the sequence.
    mRemainingDialogueLines = numLines;
    mDialogueUsesFidgets = playFidgets;
    mDialogueFinishCallbacks.push_back(finishCallback);

    // Play next line.
    PlayNextDialogueLine();
}

void DialogueManager::TriggerDialogueCue()
{
    // If we've done all the lines of dialogue we're interested in...
    if(mRemainingDialogueLines <= 0)
    {
        // If doing a dialogue that isn't using fidgets, clear the speaker before continuing.
        // This ensures that the speaker doesn't revert to doing listen fidgets due to logic that may execute in SetSpeaker for the next speaker.
        if(!mDialogueUsesFidgets)
        {
            mSpeaker.clear();
        }

        // Call finish callback.
        CallDialogueFinishedCallback();
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
        // When in a conversation, the behavior appears to be to only transition to listen fidget if one was specified for the conversation.
        bool playListenFidget = true;
        if(!mConversation.empty())
        {
            playListenFidget = false;
            for(auto& pair : mSavedListenFidgets)
            {
                if(StringUtil::EqualsIgnoreCase(pair.first->GetNoun(), mSpeaker))
                {
                    playListenFidget = true;
                }
            }
        }

        // Play listen fidget if desired.
        if(playListenFidget)
        {
            GKActor* actor = gSceneManager.GetScene()->GetActorByNoun(mSpeaker);
            if(actor != nullptr && actor->GetCurrentFidgetType() == GKActor::FidgetType::Talk)
            {
                actor->StartFidget(GKActor::FidgetType::Listen);
            }
        }
    }

    // Set new speaker.
    mSpeaker = noun;

    // Have the new speaker play talk animation.
    isUnknownSpeaker = StringUtil::EqualsIgnoreCase(mSpeaker, "UNKNOWN");
    if(mDialogueUsesFidgets && !isUnknownSpeaker)
    {
        // When in a conversation, the behavior appears to be to only transition to talk fidget if one was specified for the conversation.
        bool playTalkFidget = true;
        if(!mConversation.empty())
        {
            playTalkFidget = false;
            for(auto& pair : mSavedTalkFidgets)
            {
                if(StringUtil::EqualsIgnoreCase(pair.first->GetNoun(), mSpeaker))
                {
                    playTalkFidget = true;
                }
            }
        }

        // Play talk fidget if desired.
        if(playTalkFidget)
        {
            GKActor* actor = gSceneManager.GetScene()->GetActorByNoun(mSpeaker);
            if(actor != nullptr && actor->GetCurrentFidgetType() == GKActor::FidgetType::Listen)
            {
                actor->StartFidget(GKActor::FidgetType::Talk);
            }
        }
    }
}

void DialogueManager::SetConversation(const std::string& conversation, const std::function<void()>& finishCallback)
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

    // Before we can fully "enter" the conversation, we need to take care of changing participant fidgets and playing enter animations.
    // This can get a bit complex, depending on how complex participant fidgets and enter anims are.

    // To start, we aren't waiting on any animations to complete before entering this conversation.
    mConversationAnimWaitCount = 0;

    // Get conversation settings, which dictate which animations to play and any new fidgets to play.
    std::vector<const SceneConversation*> conversationSettings = gSceneManager.GetScene()->GetSceneData()->GetConversationSettings(conversation);

    // Figure out how many fidget interrupts we need to perform.
    // We need to count them all out *BEFORE* actually doing the interrupts, in case any interrupts execute the callback immediately.
    std::vector<GKActor*> conversationActors;
    for(const SceneConversation* settings : conversationSettings)
    {
        // An actor will play an interrupt anim if non-null and has either a talk or listen fidget for this conversation.
        GKActor* actor = gSceneManager.GetScene()->GetActorByNoun(settings->actorName);
        if(actor != nullptr && (settings->talkGas != nullptr || settings->listenGas != nullptr))
        {
            ++mConversationAnimWaitCount;
        }
        conversationActors.push_back(actor);
    }

    // We now know how many interrupts need to be performed. Iterate again and actually DO the interrupts.
    for(int i = 0; i < conversationSettings.size(); ++i)
    {
        const SceneConversation* settings = conversationSettings[i];
        GKActor* actor = conversationActors[i];
        if(actor != nullptr && (settings->talkGas != nullptr || settings->listenGas != nullptr))
        {
            // Perform the interrupt.
            // Depending on current state, this could resolve immediately, or it could take several seconds to play cleanup anims, walk to interrupt spot, etc.
            actor->InterruptFidget(true, [this, settings, actor](){

                // If this conversation has overrides for talk/listen fidgets, start using them.
                // We also save which talk/listen fidgets are being overwritten so we can revert them after the conversation is over.
                if(settings->talkGas != nullptr)
                {
                    mSavedTalkFidgets.emplace_back(actor, actor->GetTalkFidget());
                    actor->SetTalkFidget(settings->talkGas);
                }
                if(settings->listenGas != nullptr)
                {
                    mSavedListenFidgets.emplace_back(actor, actor->GetListenFidget());
                    actor->SetListenFidget(settings->listenGas);
                }

                // Start in listen fidget, unless it's null in which case fall back on talk fidget.
                // If both are null, actor just keeps doing the idle fidget I guess.
                GKActor::FidgetType fidgetType = GKActor::FidgetType::Idle;
                if(settings->listenGas != nullptr)
                {
                    fidgetType = GKActor::FidgetType::Listen;
                }
                else if(settings->talkGas != nullptr)
                {
                    fidgetType = GKActor::FidgetType::Talk;
                }

                // Decrement because we finished one "interrupt" anims.
                --mConversationAnimWaitCount;

                // But if we ALSO have an enter anim, we now need to play that!
                if(settings->enterAnim != nullptr)
                {
                    // Enter anims are pretty straightforward - just add a wait, play it, and decrement a wait when done.
                    ++mConversationAnimWaitCount;
                    gSceneManager.GetScene()->GetAnimator()->Start(settings->enterAnim, [this, actor, fidgetType](){
                        actor->StartFidget(fidgetType);
                        --mConversationAnimWaitCount;
                        CheckConversationAnimFinishCallback();
                    });
                }
                else
                {
                    actor->StartFidget(fidgetType);

                    // See if we're done entering the conversation.
                    CheckConversationAnimFinishCallback();
                }
            });
        }
    }

    // If there were no conversation settings, or no fidget changes or enter anims, we may be able to finish entering the conversation right away.
    CheckConversationAnimFinishCallback();
}

void DialogueManager::EndConversation(const std::function<void()>& finishCallback)
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

    // Play any exit anims for actors in this conversation.
    mConversationAnimWaitCount = 0;
    std::vector<const SceneConversation*> conversationSettings = gSceneManager.GetScene()->GetSceneData()->GetConversationSettings(mConversation);
    for(auto& settings : conversationSettings)
    {
        GKActor* actor = gSceneManager.GetScene()->GetActorByNoun(settings->actorName);

        // Play exit anim.
        if(settings->exitAnim != nullptr)
        {
            ++mConversationAnimWaitCount;
            gSceneManager.GetScene()->GetAnimator()->Start(settings->exitAnim, [this, actor](){
                if(actor != nullptr)
                {
                    actor->StartFidget(GKActor::FidgetType::Idle);
                }

                --mConversationAnimWaitCount;
                CheckConversationAnimFinishCallback();
            });
        }
        else
        {
            if(actor != nullptr)
            {
                actor->StartFidget(GKActor::FidgetType::Idle);
            }
        }

        // Have the actor go back to their idle fidget.
        // We don't know all participants in a conversation - that data isn't stored in SIF or anything :P
        // But if we have a conversation setting for an actor, at least we know that.


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

    // NOTE: You might think we could early out here if an action skip was occurring, and save some time/effort.
    // NOTE: However, doing so could skip important anim nodes in the YAK file (such as starting a soundtrack) that should be executed even during an action skip.

    // Construct YAK name from stored plate/sequence number.
    std::string yakName = mDialogueLicensePlate;
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
    Animation* yak = gAssetManager.LoadYak(Localizer::GetLanguagePrefix() + yakName, AssetScope::Scene);
    if(yak == nullptr)
    {
        printf("Couldn't load yak %s%s - falling back on English (E%s).\n", Localizer::GetLanguagePrefix().c_str(), yakName.c_str(), yakName.c_str());

        // Attempt to load English version.
        yak = gAssetManager.LoadYak("E" + yakName, AssetScope::Scene);
        if(yak == nullptr)
        {
            printf("Couldn't load yak %s - skipping to next dialogue line.\n", yakName.c_str());
            TriggerDialogueCue();
            return;
        }
    }

    // Play the YAK.
    // To trigger the next line of dialogue, YAKs contain a DIALOGUECUE, which causes "TriggerDialogueCue" to be called.
    AnimParams yakAnimParams;
    yakAnimParams.animation = yak;
    yakAnimParams.isYak = true;

    // It's important to get the "active" animator here, since dialogues are one of the few (only?) instances where animations can play while the scene is paused.
    // For example, if you interact with items in the inventory, the scene is paused, but dialogue still needs to play - the global animator should be used if the scene one is paused.
    Scene::GetActiveAnimator()->Start(yakAnimParams);
}

void DialogueManager::CallDialogueFinishedCallback()
{
    if(!mDialogueFinishCallbacks.empty())
    {
        if(mDialogueFinishCallbacks[0] != nullptr)
        {
            mDialogueFinishCallbacks[0]();
        }
        mDialogueFinishCallbacks.erase(mDialogueFinishCallbacks.begin());
    }
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
