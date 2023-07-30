#include "SheepAPI_Actors.h"

#include "AssetManager.h"
#include "FaceController.h"
#include "GKActor.h"
#include "ReportManager.h"
#include "SceneManager.h"

using namespace std;

shpvoid Blink(const std::string& actorName)
{
    GKActor* actor = gSceneManager.GetScene()->GetActorByNoun(actorName);
    if(actor != nullptr)
    {
        actor->GetFaceController()->Blink();
    }
    else
    {
        ExecError();
    }
    return 0;
}
RegFunc1(Blink, void, string, IMMEDIATE, REL_FUNC);

shpvoid BlinkX(const std::string& actorName, const std::string& blinkAnim)
{
    GKActor* actor = gSceneManager.GetScene()->GetActorByNoun(actorName);
    if(actor != nullptr)
    {
        actor->GetFaceController()->Blink(blinkAnim);
    }
    else
    {
        ExecError();
    }
    return 0;
}
RegFunc2(BlinkX, void, string, string, IMMEDIATE, REL_FUNC);

shpvoid EnableEyeJitter(const std::string& actorName)
{
    GKActor* actor = gSceneManager.GetScene()->GetActorByNoun(actorName);
    if(actor != nullptr)
    {
        actor->GetFaceController()->SetEyeJitterEnabled(true);
    }
    else
    {
        ExecError();
    }
    return 0;
}
RegFunc1(EnableEyeJitter, void, string, IMMEDIATE, REL_FUNC);

shpvoid DisableEyeJitter(const std::string& actorName)
{
    GKActor* actor = gSceneManager.GetScene()->GetActorByNoun(actorName);
    if(actor != nullptr)
    {
        actor->GetFaceController()->SetEyeJitterEnabled(false);
    }
    else
    {
        ExecError();
    }
    return 0;
}
RegFunc1(DisableEyeJitter, void, string, IMMEDIATE, REL_FUNC);

shpvoid EyeJitter(const std::string& actorName)
{
    GKActor* actor = gSceneManager.GetScene()->GetActorByNoun(actorName);
    if(actor != nullptr)
    {
        actor->GetFaceController()->EyeJitter();
    }
    else
    {
        ExecError();
    }
    return 0;
}
RegFunc1(EyeJitter, void, string, IMMEDIATE, REL_FUNC);

/*
shpvoid Glance(std::string actorName, int percentX, int percentY, int durationMs)
{
    return 0;
}

shpvoid GlanceX(std::string actorName, int leftPercentX, int leftPercentY,
                int rightPercentX, int rightPercentY, int durationMs)
{
    return 0;
}
*/

// SetEyeOffsets

shpvoid SetMood(const std::string& actorName, const std::string& moodName)
{
    // Get actor and make sure it's valid.
    GKActor* actor = gSceneManager.GetScene()->GetActorByNoun(actorName);
    if(actor == nullptr)
    {
        ExecError();
        return 0;
    }

    // Apply mood.
    //TODO: What if mood is invalid?
    actor->GetFaceController()->SetMood(moodName);
    return 0;
}
RegFunc2(SetMood, void, string, string, IMMEDIATE, REL_FUNC);

shpvoid ClearMood(const std::string& actorName)
{
    // Get actor and make sure it's valid.
    GKActor* actor = gSceneManager.GetScene()->GetActorByNoun(actorName);
    if(actor == nullptr)
    {
        ExecError();
        return 0;
    }

    // Clear mood.
    actor->GetFaceController()->ClearMood();
    return 0;
}
RegFunc1(ClearMood, void, string, IMMEDIATE, REL_FUNC);

shpvoid Expression(const std::string& actorName, const std::string& expression)
{
    GKActor* actor = gSceneManager.GetScene()->GetActorByNoun(actorName);
    if(actor != nullptr)
    {
        actor->GetFaceController()->DoExpression(expression);
    }
    else
    {
        ExecError();
    }
    return 0;
}
RegFunc2(Expression, void, string, string, IMMEDIATE, REL_FUNC);

shpvoid SetIdleGAS(const std::string& actorName, const std::string& gasName)
{
    // Get actor and make sure it's valid.
    GKActor* actor = gSceneManager.GetScene()->GetActorByNoun(actorName);
    if(actor == nullptr)
    {
        ExecError();
        return 0;
    }

    // If gas is empty, that means clear out fidget (not an error).
    if(gasName.empty())
    {
        actor->SetIdleFidget(nullptr);
        return 0;
    }

    // Load the fidget.
    // If the fidget doesn't exist, we still set it, but we output an error.
    GAS* fidget = gAssetManager.LoadGAS(gasName, AssetScope::Scene);
    if(fidget == nullptr)
    {
        gReportManager.Log("Error", "Attempted to load an invalid fidget file: " + gasName);
    }
    actor->SetIdleFidget(fidget);
    return 0;
}
RegFunc2(SetIdleGAS, void, string, string, WAITABLE, REL_FUNC); // NOTE: function is waitable, but does nothing.

shpvoid SetListenGAS(const std::string& actorName, const std::string& gasName)
{
    // Get actor and make sure it's valid.
    GKActor* actor = gSceneManager.GetScene()->GetActorByNoun(actorName);
    if(actor == nullptr)
    {
        ExecError();
        return 0;
    }

    // If gas is empty, that means clear out fidget (not an error).
    if(gasName.empty())
    {
        actor->SetListenFidget(nullptr);
        return 0;
    }

    // Load the fidget.
    // If the fidget doesn't exist, we still set it, but we output an error.
    GAS* fidget = gAssetManager.LoadGAS(gasName, AssetScope::Scene);
    if(fidget == nullptr)
    {
        gReportManager.Log("Error", "Attempted to load an invalid fidget file: " + gasName);
    }
    actor->SetListenFidget(fidget);
    return 0;
}
RegFunc2(SetListenGAS, void, string, string, WAITABLE, REL_FUNC); // NOTE: function is waitable, but does nothing.

shpvoid SetTalkGAS(const std::string& actorName, const std::string& gasName)
{
    // Get actor and make sure it's valid.
    GKActor* actor = gSceneManager.GetScene()->GetActorByNoun(actorName);
    if(actor == nullptr)
    {
        ExecError();
        return 0;
    }

    // If gas is empty, that means clear out fidget (not an error).
    if(gasName.empty())
    {
        actor->SetTalkFidget(nullptr);
        return 0;
    }

    // Load the fidget.
    // If the fidget doesn't exist, we still set it, but we output an error.
    GAS* fidget = gAssetManager.LoadGAS(gasName, AssetScope::Scene);
    if(fidget == nullptr)
    {
        gReportManager.Log("Error", "Attempted to load an invalid fidget file: " + gasName);
    }
    actor->SetTalkFidget(fidget);
    return 0;
}
RegFunc2(SetTalkGAS, void, string, string, WAITABLE, REL_FUNC); // NOTE: function is waitable, but does nothing.

shpvoid StartIdleFidget(const std::string& actorName)
{
    GKActor* actor = gSceneManager.GetScene()->GetActorByNoun(actorName);
    if(actor == nullptr)
    {
        ExecError();
        return 0;
    }

    actor->StartFidget(GKActor::FidgetType::Idle);
    return 0;
}
RegFunc1(StartIdleFidget, void, string, WAITABLE, REL_FUNC);

shpvoid StartListenFidget(const std::string& actorName)
{
    GKActor* actor = gSceneManager.GetScene()->GetActorByNoun(actorName);
    if(actor == nullptr)
    {
        ExecError();
        return 0;
    }

    actor->StartFidget(GKActor::FidgetType::Listen);
    return 0;
}
RegFunc1(StartListenFidget, void, string, WAITABLE, REL_FUNC);

shpvoid StartTalkFidget(const std::string& actorName)
{
    GKActor* actor = gSceneManager.GetScene()->GetActorByNoun(actorName);
    if(actor == nullptr)
    {
        ExecError();
        return 0;
    }

    actor->StartFidget(GKActor::FidgetType::Talk);
    return 0;
}
RegFunc1(StartTalkFidget, void, string, WAITABLE, REL_FUNC);

shpvoid StopFidget(const std::string& actorName)
{
    GKActor* actor = gSceneManager.GetScene()->GetActorByNoun(actorName);
    if(actor == nullptr)
    {
        ExecError();
    }
    else
    {
        actor->StopFidget(AddWait());
    }
    return 0;
}
RegFunc1(StopFidget, void, string, WAITABLE, REL_FUNC);

shpvoid ActionWaitClearRegion(const std::string& actorName, int regionId, float destAccuracy, const std::string& exitPosition)
{
    // Get the actor.
    GKActor* actor = gSceneManager.GetScene()->GetActorByNoun(actorName);
    if(actor == nullptr)
    {
        ExecError();
        return 0;
    }

    // Get the scene position.
    const ScenePosition* scenePosition = gSceneManager.GetScene()->GetPosition(exitPosition);
    if(scenePosition == nullptr)
    {
        ExecError();
        return 0;
    }

    // Clear the area!
    actor->GetWalker()->WalkOutOfRegion(regionId, scenePosition->position, scenePosition->heading, AddWait());
    return 0;
}
RegFunc4(ActionWaitClearRegion, void, string, int, float, string, WAITABLE, REL_FUNC);

shpvoid ClearRegion(const std::string& actorName, int regionId, float destAccuracy, const std::string& exitPosition)
{
    // Seems identical?
    ActionWaitClearRegion(actorName, regionId, destAccuracy, exitPosition);
    return 0;
}
RegFunc4(ClearRegion, void, string, int, float, string, WAITABLE, REL_FUNC);

shpvoid SetWalkAnim(const std::string& actorName, const std::string& start, const std::string& cont,
                    const std::string& startTurnLeft, const std::string& startTurnRight)
{
    // Get the Actor.
    GKActor* actor = gSceneManager.GetScene()->GetActorByNoun(actorName);
    if(actor == nullptr)
    {
        ExecError();
        return 0;
    }

    // Load start/loop anims. Neither is optional.
    Animation* startAnim = gAssetManager.LoadAnimation(start, AssetScope::Scene);
    Animation* loopAnim = gAssetManager.LoadAnimation(cont, AssetScope::Scene);
    if(startAnim == nullptr || loopAnim == nullptr)
    {
        ExecError();
        return 0;
    }

    // Turn anims are optional. We only throw errors if they are specified and not found.
    Animation* startTurnLeftAnim = nullptr;
    if(!startTurnLeft.empty())
    {
        startTurnLeftAnim = gAssetManager.LoadAnimation(startTurnLeft, AssetScope::Scene);
        if(startTurnLeftAnim == nullptr)
        {
            ExecError();
            return 0;
        }
    }
    Animation* startTurnRightAnim = nullptr;
    if(!startTurnRight.empty())
    {
        startTurnRightAnim  = gAssetManager.LoadAnimation(startTurnRight, AssetScope::Scene);
        if(startTurnRightAnim == nullptr)
        {
            ExecError();
            return 0;
        }
    }

    actor->GetWalker()->SetWalkAnims(startAnim, loopAnim, startTurnLeftAnim, startTurnRightAnim);
    return 0;
}
RegFunc5(SetWalkAnim, void, string, string, string, string, string, IMMEDIATE, REL_FUNC);

 /*
shpvoid TurnHead(std::string actorName, int percentX, int percentY, int durationMs)
{
 std::cout << "TurnHead" << std::endl;
 return 0;
}
RegFunc4(TurnHead, void, string, int, int, int, WAITABLE, REL_FUNC);

shpvoid TurnToModel(std::string actorName, std::string modelName)
{
 std::cout << "TurnToModel" << std::endl;
 return 0;
}
RegFunc2(TurnToModel, void, string, string, WAITABLE, REL_FUNC);

shpvoid LookitActor(std::string actorName, std::string otherActorName,
                     std::string componentsSpec, float durationSec)
{
    return 0;
}
RegFunc4(LookitActor, void, string, string, string, float, IMMEDIATE, REL_FUNC);

shpvoid LookitActorQuick(std::string actorName, std::string otherActorName,
                         std::string componentsSpec, float durationSec)
{
    return 0;
}
RegFunc4(LookitActorQuick, void, string, string, string, float, IMMEDIATE, REL_FUNC);

shpvoid LookitModel(std::string actorName, std::string modelName,
                    std::string componentsSpec, float durationSec)
{
    return 0;
}
RegFunc4(LookitModel, void, string, string, string, float, IMMEDIATE, REL_FUNC);

shpvoid LookitModelQuick(std::string actorName, std::string modelName,
                         std::string componentsSpec, float durationSec)
{
    return 0;
}
RegFunc4(LookitModelQuick, void, string, string, string, float, IMMEDIATE, REL_FUNC);

shpvoid LookitModelX(std::string actorName, std::string modelName, int mesh,
                     std::string boxModifier, float offsetX, float offsetY, float offsetZ,
                     std::string componentsSpec, float durationSec)
{
    return 0;
}
//TODO: Need a new RegFuncX for 9 arguments!

//LookitModelQuickX

//LookitNoun

//LookitNounQuick

//LookitPoint

//LookitCameraAngle

//LookitMouse

//LookitPlayer

//LookitCancel
*/
