//
// GasNodes.h
//
// Clark Kromenaker
//
// Various nodes that are used in constructing GAS scripts.
// Similar idea to Animation system's AnimNodes.
//
#pragma once
#include <string>
#include <vector>

#include "Vector3.h"

class Animation;
class GAS;
class GasPlayer;

// Base struct for all GAS nodes.
struct GasNode
{
    // Chance that this node will execute. 0 = no chance, 100 = definitely
    int random = 100;

    virtual ~GasNode() { }
    virtual float Execute(GasPlayer* player) = 0;
};

// A node that plays an animation.
struct AnimGasNode : public GasNode
{
    // Animation to do.
    Animation* animation = nullptr;

    // If true, relative animations can move the affected model.
    bool moving = false;

    float Execute(GasPlayer* player) override;
};

// A node that plays "one of" a set of animation nodes.
struct OneOfGasNode : public GasNode
{
    // List of nodes to choose from.
    std::vector<AnimGasNode*> animNodes;

    ~OneOfGasNode() override;
    float Execute(GasPlayer* player) override;
};

// A node that waits some amount of time before continuing.
struct WaitGasNode : public GasNode
{
    // Range of time to wait.
    float minWaitTimeSeconds = 0.0f;
    float maxWaitTimeSeconds = 0.0f;

    float Execute(GasPlayer* player) override;
};

// A node that goes to some other node.
// Note that LOOP is just a goto with index 0.
struct GotoGasNode : public GasNode
{
    // Node index this goes to.
    int index = 0;

    float Execute(GasPlayer* player) override;
};

// A node that sets a variable's value.
struct SetGasNode : public GasNode
{
    char varName = 'A';
    int value = 0;

    float Execute(GasPlayer* player) override;
};

// A node that increments a variable's value.
struct IncGasNode : public GasNode
{
    char varName = 'A';
    
    float Execute(GasPlayer* player) override;
};

// A node that decrements a variable's value.
struct DecGasNode : public GasNode
{
    char varName = 'A';

    float Execute(GasPlayer* player) override;
};

// A node that goes to a label if a variable's value meets a condition.
struct IfGasNode : public GasNode
{
    enum class Operation : char
    {
        Equals,
        LessThan,
        GreaterThan
    };

    // If varName [=, <, >] value...
    char varName = 'A';
    Operation operation = Operation::Equals;
    int value = 0;

    // ...go to this index!
    int index = 0;

    float Execute(GasPlayer* player) override;
};

// A node that tells actor to walk to a position.
struct WalkToGasNode : public GasNode
{
    // Can specify a position name OR an X/Y/Z pos.
    // Name takes precendent (used if not empty).
    std::string positionName;
    Vector3 position;

    float Execute(GasPlayer* player) override;
};

// A node that tells actor to walk to one of the positions in the list.
struct ChooseWalkGasNode : public GasNode
{
    // Randomly chooses one of these to walk to.
    std::vector<std::string> positionNames;

    float Execute(GasPlayer* player) override;
};

// A node that sets an "interrupt position" for where model walks to when their walk path is interrupted.
// For example, if actor is patrolling a path and Ego wants to talk, they'll go to this spot before talk begins.
struct UseIPosGasNode : public GasNode
{
    std::string positionName;

    float Execute(GasPlayer* player) override;
};

// A node that defines an animation "cleanup" pair.
struct UseCleanupGasNode : public GasNode
{
    Animation* animationNeedingCleanup = nullptr;
    Animation* animationDoingCleanup = nullptr;

    float Execute(GasPlayer* player) override;
};

//TODO: UseNewIdle (used occasionally, maybe important)

// A node that defines an interrupt spot to use specifically for talking.
struct UseTalkIPosGasNode : public GasNode
{
    std::string positionName;

    float Execute(GasPlayer* player) override;
};

// A node that defines an animation "cleanup" pair to use specifically when talking.
struct UseTalkCleanupGasNode : public GasNode
{
    Animation* animationNeedingCleanup = nullptr;
    Animation* animationDoingCleanup = nullptr;

    float Execute(GasPlayer* player) override;
};

//TODO: UseTalkNewIdle (used occasionally, maybe important)

//TODO: UseClearMood (never used in the game)
//TODO: UseTalkClearMood (never used in the game)

// A node that switches the actor to a new GAS script.
//TODO: Despite the NEWIDLE keyword used in scripts, a better name for this might be like "ChangeScriptGasNode" or something.
struct NewIdleGasNode : public GasNode
{
    GAS* newGas = nullptr;

    float Execute(GasPlayer* player) override;
};

// A node that goes to a label when the actor is near (or not near) the object with the specified noun.
struct WhenNearGasNode : public GasNode
{
    // If true, the condition is flipped to "when not near" rather than "when near".
    bool notNear = false;

    // When I am more than X distance away from this noun...
    float distance = 0.0f;
    std::string noun;

    // If empty, the distance is from ME to NOUN
    // If set, the distance is from NOUN to OTHER_NOUN.
    std::string otherNoun;

    // ...go to this index in the autoscript (derived from a label).
    int index = 0;

    float Execute(GasPlayer* player) override;
    bool CheckCondition(GasPlayer* player);
};

//TODO: WhenInView (rarely used, not mission critical)

// A node that will play a line of dialog.
struct DialogueGasNode : public GasNode
{
    Animation* yakAnimation = nullptr;

    float Execute(GasPlayer* player) override;
};

// A node that changes the location of the actor running the autoscript.
struct LocationGasNode : public GasNode
{
    std::string location;

    float Execute(GasPlayer* player) override;
};

//TODO: SetMood (only used once, not mission critical)

//TODO: ClearMood (never used in game)

//TODO: Glance (never used in game)

//TODO: TurnHead (never used in game)

//TODO: LookAt (used occasionally, not mission critical)

//TODO: Expression (never used in game)