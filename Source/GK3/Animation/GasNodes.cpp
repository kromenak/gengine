#include "GasNodes.h"

#include "Animator.h"
#include "GasPlayer.h"
#include "GKActor.h"
#include "LocationManager.h"
#include "Random.h"
#include "SceneManager.h"

float AnimGasNode::Execute(GasPlayer* player)
{
    // Do random check. If it fails, we don't execute.
    // But note execution count is still incremented!
    int randomCheck = Random::Range(1, 101);
    if(randomCheck > random) { return 0; }

    // Must have a valid animation, and GasPlayer, and Animator.
    if(animation == nullptr) { return 0; }

    // Play the animation!
    int executionCounter = player->GetExecutionCounter();
    player->StartAnimation(animation, [player, executionCounter](){
        player->NextNode(executionCounter);
    });

    // Return -1 to disable timer system and just wait for callback.
    return -1.0f;
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
    if(animNodes.empty()) { return 0.0f; }
    return animNodes[Random::Range(0, animNodes.size())]->Execute(player);
}

float WaitGasNode::Execute(GasPlayer* player)
{
    // Do random check. If it fails, we don't execute.
    // But note execution count is still incremented!
    int randomCheck = Random::Range(1, 101);
    if(randomCheck > random) { return 0.0f; }

    // We will execute this node. Decide wait time based on min/max.
    if(minWaitTimeSeconds == maxWaitTimeSeconds) { return minWaitTimeSeconds; }
    if(maxWaitTimeSeconds == 0 && minWaitTimeSeconds > 0) { return minWaitTimeSeconds; }
    if(maxWaitTimeSeconds != 0 && minWaitTimeSeconds > maxWaitTimeSeconds) { return minWaitTimeSeconds; }

    // Normal case - random between min and max.
    return Random::Range(minWaitTimeSeconds, maxWaitTimeSeconds);
}

float GotoGasNode::Execute(GasPlayer* player)
{
    //std::cout << player->GetOwner()->GetName() << " Goto " << index << std::endl;
    player->SetNodeIndex(index);
    return 0.0f;
}

float SetGasNode::Execute(GasPlayer* player)
{
    //std::cout << player->GetOwner()->GetName() << " Set Var " << varName << " to " << value << std::endl;
    player->SetVar(varName, value);
    return 0.0f;
}

float IncGasNode::Execute(GasPlayer* player)
{
    player->SetVar(varName, player->GetVar(varName) + 1);
    //std::cout << player->GetOwner()->GetName() << " Inc Var " << varName << " to " << player->GetVar(varName) << std::endl;
    return 0.0f;
}

float DecGasNode::Execute(GasPlayer* player)
{
    player->SetVar(varName, player->GetVar(varName) - 1);
    //std::cout << player->GetOwner()->GetName() << " Dec Var " << varName << " to " << player->GetVar(varName) << std::endl;
    return 0.0f;
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
        player->SetNodeIndex(index);
    }
    return 0.0f;
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
        const ScenePosition* scenePosition = gSceneManager.GetScene()->GetPosition(positionName);
        if(scenePosition != nullptr)
        {
            walkToPos = scenePosition->position;
            walkToHeading = scenePosition->heading;
        }
    }

    // Start walk to.
    // Must use special "gas" version, or the walk/animation system will cause the current GAS script to be stopped/restarted.
    int executionCounter = player->GetExecutionCounter();
    actor->WalkToGas(walkToPos, walkToHeading, [player, executionCounter](){
        player->NextNode(executionCounter);
    });

    // Return -1 to disable timer system and just wait for callback.
    return -1.0f;
}

float ChooseWalkGasNode::Execute(GasPlayer* player)
{
    if(positionNames.size() == 0) { return 0; }

    GKActor* actor = static_cast<GKActor*>(player->GetOwner());
    if(actor == nullptr) { return 0; }

    // Randomly choose a position from the list.
    int randomIndex = Random::Range(0, positionNames.size());
    size_t counter = 0;
    const ScenePosition* scenePosition = nullptr;
    while(counter < positionNames.size())
    {
        const ScenePosition* candidate = gSceneManager.GetScene()->GetPosition(positionNames[randomIndex]);
        if(candidate != nullptr && !actor->GetWalker()->AtPosition(candidate->position))
        {
            // Found a position that works!
            scenePosition = candidate;
            break;
        }

        // Either candidate was null (unlikely) or actor's already at that position (more likely).
        // Either way, we can't use the randomly selected position! Just increment forward until we find one that works...
        randomIndex = (randomIndex + 1) % positionNames.size();
        ++counter;
    }

    // At this point, if we don't have a position to use, we have to just skip this node.
    // Means no walk position was usable.
    if(scenePosition == nullptr) { return 0; }

    // Start walk to.
    int executionCounter = player->GetExecutionCounter();
    actor->WalkToGas(scenePosition->position, scenePosition->heading, [player, executionCounter](){
        player->NextNode(executionCounter);
    });

    // Return -1 to disable timer system and just wait for callback.
    return -1.0f;
}

float UseIPosGasNode::Execute(GasPlayer* player)
{
    // We'll just assume the position name provided is valid - or else null is set.
    if(forTalk)
    {
        player->SetTalkInterruptPosition(gSceneManager.GetScene()->GetPosition(positionName));
        player->SetTalkInterruptAnimation(animation);
    }
    else
    {
        player->SetInterruptPosition(gSceneManager.GetScene()->GetPosition(positionName));
        player->SetInterruptAnimation(animation);
    }
    return 0.0f;
}

float UseCleanupGasNode::Execute(GasPlayer* player)
{
    if(animationNeedingCleanup != nullptr && animationDoingCleanup != nullptr)
    {
        if(forTalk)
        {
            player->SetTalkCleanup(animationNeedingCleanup, animationDoingCleanup);
        }
        else
        {
            player->SetCleanup(animationNeedingCleanup, animationDoingCleanup);
        }
    }
    return 0.0f;
}

float UseNewIdleGasNode::Execute(GasPlayer* player)
{
    if(forTalk)
    {
        player->SetNewIdleOnTalkAction(newGas);
    }
    else
    {
        player->SetNewIdleOnAction(newGas);
    }
    return 0.0f;
}

float UseClearFlagGasNode::Execute(GasPlayer* player)
{
    if(forTalk)
    {
        player->SetTalkInterruptClearFlag(clearFlag);
    }
    else
    {
        player->SetInterruptClearFlag(clearFlag);
    }
    return 0.0f;
}

float NewIdleGasNode::Execute(GasPlayer* player)
{
    // Stop the current autoscript first to ensure that cleanup animations execute.
    // Then play the new autoscript.
    player->Stop([this, player](){
        player->Play(newGas);
    });
    return 0.0f;
}

float WhenNearGasNode::Execute(GasPlayer* player)
{
    player->AddDistanceCondition(this);
    return 0.0f;
}

bool WhenNearGasNode::CheckCondition(GasPlayer* player)
{
    // Get actor associated with first noun. Fail out if not found.
    Actor* objectA = gSceneManager.GetScene()->GetSceneObjectByNoun(noun);
    if(objectA == nullptr) { return false; }

    // Get other actor. If "otherNoun" is not empty, the other actor is obtained same way.
    // If it is empty, the other actor is "us" (whoever is running the GAS script).
    Actor* objectB = nullptr;
    if(!otherNoun.empty())
    {
        objectB = gSceneManager.GetScene()->GetSceneObjectByNoun(otherNoun);
    }
    else
    {
        objectB = player->GetOwner();
    }
    if(objectB == nullptr) { return false; }

    // Get distance between objects.
    float distSq = (objectA->GetWorldPosition() - objectB->GetWorldPosition()).GetLengthSq();
    float checkDistSq = distance * distance;

    // See if condition is met.
    // If "notNear" is set, then condition is met when we are farther away than the required distance.
    // Otherwise, the condition is met when we are closer than the required distance.
    bool conditionMet = false;
    if(notNear && distSq >= checkDistSq)
    {
        conditionMet = true;
    }
    else if(!notNear && distSq <= checkDistSq)
    {
        conditionMet = true;
    }

    // Return true to tell caller whether this condition was met.
    return conditionMet;
}

float DialogueGasNode::Execute(GasPlayer* player)
{
    // Just play the thing! Use callback method for signaling when done.
    AnimParams params;
    params.animation = yakAnimation;
    params.isYak = true;

    int executionCounter = player->GetExecutionCounter();
    gSceneManager.GetScene()->GetAnimator()->Start(params, [player, executionCounter](){
        player->NextNode(executionCounter);
    });
    return -1.0f;
}

float LocationGasNode::Execute(GasPlayer* player)
{
    GKObject* owner = static_cast<GKObject*>(player->GetOwner());
    gLocationManager.SetActorLocation(owner->GetNoun(), location);
    return 0.0f;
}