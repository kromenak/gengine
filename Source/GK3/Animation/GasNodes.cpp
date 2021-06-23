#include "GasNodes.h"

#include "Animator.h"
#include "GasPlayer.h"
#include "GEngine.h"
#include "GKActor.h"
#include "Scene.h"

float AnimGasNode::Execute(GasPlayer* player)
{
    // Do random check. If it fails, we don't execute.
    // But note execution count is still incremented!
    int randomCheck = rand() % 100 + 1;
    if(randomCheck > random) { return 0; }

    // Must have a valid animation, and GasPlayer, and Animator.
    if(animation == nullptr) { return 0; }

    // Play the animation!
    AnimParams animParams;
    animParams.animation = animation;
    animParams.fromAutoScript = true;
    GEngine::Instance()->GetScene()->GetAnimator()->Start(animParams);

    // HACK: When the EXACT anim duration is used, pure looping anims (like lobby fans) have a noticeable pause between loops.
    // To combat this, let's try reducing the anim duration just a tiny bit.
    // Unclear whether this will cause problems elsewhere...may have to revisit.
    // Also, is the pause due to the way the anim is authored, or is it caused by a calculation error somewhere?
    const float kAnimDurationFudge = 0.05f;
    float duration = animation->GetDuration() - kAnimDurationFudge;

    //std::cout << "Playing animation " << animation->GetName() << " for " << duration << " seconds." << std::endl;
    return duration;
}

OneOfGasNode::~OneOfGasNode()
{
    for(auto& node : animNodes)
    {
        delete node;
    }
}

float OneOfGasNode::Execute(GasPlayer* player)
{
    if(animNodes.size() == 0) { return 0; }
    int randomIndex = rand() % animNodes.size();
    return animNodes[randomIndex]->Execute(player);
}

float WaitGasNode::Execute(GasPlayer* player)
{
    // Do random check. If it fails, we don't execute.
    // But note execution count is still incremented!
    int randomCheck = rand() % 100 + 1;
    if(randomCheck > random) { return 0; }

    // We will execute this node. Decide wait time based on min/max.
    if(minWaitTimeSeconds == maxWaitTimeSeconds) { return minWaitTimeSeconds * 1000; }
    if(maxWaitTimeSeconds == 0 && minWaitTimeSeconds > 0) { return minWaitTimeSeconds * 1000; }
    if(maxWaitTimeSeconds != 0 && minWaitTimeSeconds > maxWaitTimeSeconds) { return minWaitTimeSeconds * 1000; }

    // Normal case - random between min and max.
    return (rand() % maxWaitTimeSeconds + minWaitTimeSeconds);
}

float GotoGasNode::Execute(GasPlayer* player)
{
    player->SetIndex(index);
    return 0;
}

float SetGasNode::Execute(GasPlayer* player)
{
    player->SetVar(varName, value);
    return 0;
}

float IncGasNode::Execute(GasPlayer* player)
{
    player->SetVar(varName, player->GetVar(varName) + 1);
    return 0;
}

float DecGasNode::Execute(GasPlayer* player)
{
    player->SetVar(varName, player->GetVar(varName) - 1);
    return 0;
}

float IfGasNode::Execute(GasPlayer* player)
{
    // See if current value of variable meets condition.
    bool meetsCondition = false;
    if(operation == Operation::Equals)
    {
        meetsCondition = (player->GetVar(varName) == value);
    }
    else if(operation == Operation::GreaterThan)
    {
        meetsCondition = (player->GetVar(varName) > value);
    }
    else if(operation == Operation::LessThan)
    {
        meetsCondition = (player->GetVar(varName) < value);
    }

    // If so, jump to the label.
    if(meetsCondition)
    {
        player->SetIndex(index);
    }
    return 0;
}

float WalkToGasNode::Execute(GasPlayer* player)
{
    GKActor* actor = static_cast<GKActor*>(player->GetOwner());
    if(actor == nullptr) { return 0; }

    // Figure out walk position.
    Vector3 walkToPos = position;
    Heading walkToHeading = Heading::None;
    if(!positionName.empty())
    {
        const ScenePosition* scenePosition = GEngine::Instance()->GetScene()->GetPosition(positionName);
        if(scenePosition != nullptr)
        {
            walkToPos = scenePosition->position;
            walkToHeading = scenePosition->heading;
        }
    }

    // Start walk to.
    actor->WalkTo(walkToPos, walkToHeading, std::bind(&GasPlayer::NextNode, player));

    // Return -1 to disable timer system and just wait for callback.
    return -1;
}

float ChooseWalkGasNode::Execute(GasPlayer* player)
{
    if(positionNames.size() == 0) { return 0; }

    GKActor* actor = static_cast<GKActor*>(player->GetOwner());
    if(actor == nullptr) { return 0; }

    // Randomly choose a position from the list.
    int randomIndex = rand() % positionNames.size();
    int counter = 0;
    const ScenePosition* scenePosition = nullptr;
    while(counter < positionNames.size())
    {
        const ScenePosition* candidate = GEngine::Instance()->GetScene()->GetPosition(positionNames[randomIndex]);
        if(candidate != nullptr && !actor->AtPosition(candidate->position))
        {
            // Found a position that works!
            scenePosition = candidate;
            break;
        }

        // Either candidate was null (unlikely) or actor's already at that position (more likely).
        // Either way, we can't use the randomly selected position! Just increment forward until we find one that works...
        randomIndex = (randomIndex + 1) % positionNames.size();
        counter++;
    }

    // At this point, if we don't have a position to use, we have to just skip this node.
    // Means no walk position was usable.
    if(scenePosition == nullptr) { return 0; }
    
    // Start walk to.
    actor->WalkTo(scenePosition->position, scenePosition->heading, std::bind(&GasPlayer::NextNode, player));

    // Return -1 to disable timer system and just wait for callback.
    return -1;
}

float UseIPosGasNode::Execute(GasPlayer* player)
{
    // We'll just assume the position name provided is valid - or else null is set.
    player->SetInterruptPosition(GEngine::Instance()->GetScene()->GetPosition(positionName));
    return 0;
}

float UseCleanupGasNode::Execute(GasPlayer* player)
{
    if(animationNeedingCleanup != nullptr && animationDoingCleanup != nullptr)
    {
        player->SetCleanup(animationNeedingCleanup, animationDoingCleanup);
    }
    return 0;
}

float UseTalkIPosGasNode::Execute(GasPlayer* player)
{
    // We'll just assume the position name provided is valid - or else null is set.
    player->SetTalkInterruptPosition(GEngine::Instance()->GetScene()->GetPosition(positionName));
    return 0;
}

float UseTalkCleanupGasNode::Execute(GasPlayer* player)
{
    if(animationNeedingCleanup != nullptr && animationDoingCleanup != nullptr)
    {
        player->SetTalkCleanup(animationNeedingCleanup, animationDoingCleanup);
    }
    return 0;
}

float NewIdleGasNode::Execute(GasPlayer* player)
{
    player->Play(newGas);
    return 0.0f;
}

float WhenNoLongerNearGasNode::Execute(GasPlayer* player)
{
    player->AddWhenNoLongerNearNode(this);
    return 0.0f;
}

bool WhenNoLongerNearGasNode::CheckCondition(GasPlayer* player)
{
    // Get actor associated with first noun. Fail out if not found.
    Actor* objectA = GEngine::Instance()->GetScene()->GetSceneObjectByNoun(noun);
    if(objectA == nullptr) { return false; }

    // Get other actor. If "otherNoun" is not empty, the other actor is obtained same way.
    // If it is empty, the other actor is "us" (whoever is running the GAS script).
    Actor* objectB = nullptr;
    if(!otherNoun.empty())
    {
        objectB = GEngine::Instance()->GetScene()->GetSceneObjectByNoun(otherNoun);
    }
    else
    {
        objectB = player->GetOwner();
    }
    if(objectB == nullptr) { return false; }

    // Get distance between objects.
    // If within distance, early out b/c we are NOT "no longer near" in that case.
    float distSq = (objectA->GetWorldPosition() - objectB->GetWorldPosition()).GetLengthSq();
    if(distSq < distance * distance) { return false; }

    // Ok, so the objects seem to be far apart, so I'd say that means "no longer near" condition is true!
    // Tell player to go to desired index.
    player->SetIndex(index);

    // Return true to tell script this condition was met and we should no longer check for this condition.
    return true;
}