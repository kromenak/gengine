//
// SheepAPI.cpp
//
// Clark Kromenaker
//
#include "SheepAPI.h"
#include "GEngine.h"
#include "Scene.h"
#include "Services.h"

// Required for macros to work correctly with "string" instead of "std::string".
using namespace std;

// Maps from function name to a function pointer.
// Each map is for a different number of arguments (0-8).
std::map<std::string, Value (*)(const Value&)> map0;
std::map<std::string, Value (*)(const Value&)> map1;
std::map<std::string, Value (*)(const Value&, const Value&)> map2;
std::map<std::string, Value (*)(const Value&, const Value&, const Value&)> map3;
std::map<std::string, Value (*)(const Value&, const Value&, const Value&,
                                const Value&)> map4;
std::map<std::string, Value (*)(const Value&, const Value&, const Value&,
                                const Value&, const Value&)> map5;
std::map<std::string, Value (*)(const Value&, const Value&, const Value&,
                                const Value&, const Value&, const Value&)> map6;
std::map<std::string, Value (*)(const Value&, const Value&, const Value&,
                                const Value&, const Value&, const Value&,
                                const Value&)> map7;
std::map<std::string, Value (*)(const Value&, const Value&, const Value&,
                                const Value&, const Value&, const Value&,
                                const Value&, const Value&)> map8;

Value CallSysFunc(const std::string& name)
{
    return map0.at(name)(0);
}

Value CallSysFunc(const std::string& name, const Value& x1)
{
    return map1.at(name)(x1);
}

Value CallSysFunc(const std::string& name, const Value& x1, const Value& x2)
{
    return map2.at(name)(x1, x2);
}

Value CallSysFunc(const std::string& name, const Value& x1, const Value& x2, const Value& x3)
{
    return map3.at(name)(x1, x2, x3);
}

Value CallSysFunc(const std::string& name, const Value& x1, const Value& x2, const Value& x3,
                  const Value& x4)
{
    return map4.at(name)(x1, x2, x3, x4);
}

Value CallSysFunc(const std::string& name, const Value& x1, const Value& x2, const Value& x3,
                  const Value& x4, const Value& x5)
{
    return map5.at(name)(x1, x2, x3, x4, x5);
}

Value CallSysFunc(const std::string& name, const Value& x1, const Value& x2, const Value& x3,
                  const Value& x4, const Value& x5, const Value& x6)
{
    return map6.at(name)(x1, x2, x3, x4, x5, x6);
}

std::vector<SysImport> sysFuncs;

void AddSysImport(const std::string& name, char retType, std::initializer_list<char> argTypes)
{
    SysImport sysImport;
    sysImport.name = name;
    sysImport.returnType = retType;
    for(auto argType : argTypes)
    {
        sysImport.argumentTypes.push_back(argType);
    }
    sysFuncs.push_back(sysImport);
}

void InitSysImports()
{
    /*
    if(sysFuncs.size() > 0) { return; }
    
    // ACTORS
    AddSysImport("Blink", 0, { 3 });
    AddSysImport("BlinkX", 0, { 3, 3 });
    
    AddSysImport("ClearMood", 0, { 3 });
    
    AddSysImport("EnableEyeJitter", 0, { 3 });
    AddSysImport("DisableEyeJitter", 0, { 3 });
    AddSysImport("EyeJitter", 0, { 3 });
    
    AddSysImport("DumpActorPosition", 0, { 3 });
    
    AddSysImport("Expression", 0, { 3, 3 });
    
    AddSysImport("GetEgoCurrentLocationCount", 1, { });
    AddSysImport("GetEgoLocationCount", 1, { 3 });
    
    AddSysImport("GetEgoName", 3, { });
    
    AddSysImport("GetIndexedPosition", 3, { 1 });
    AddSysImport("GetPositionCount", 1, { });
    
    AddSysImport("Glance", 0, { 3, 1, 1, 1 });
    AddSysImport("GlanceX", 0, { 3, 1, 1, 1, 1, 1 });
    
    AddSysImport("InitEgoPosition", 0, { 3 });
    
    AddSysImport("IsActorAtLocation", 1, { 3, 3 });
    AddSysImport("IsActorNear", 1, { 3, 3, 2 });
    AddSysImport("IsWalkingActorNear", 1, { 3, 3, 2 });
    
    AddSysImport("IsActorOffstage", 1, { 3 });
    
    AddSysImport("IsCurrentEgo", 1, { 3 });
    
    AddSysImport("LookitActor", 0, { 3, 3, 3, 2 });
    AddSysImport("LookitActorQuick", 0, { 3, 3, 3, 2 });
    
    AddSysImport("LookitModel", 0, { 3, 3, 3, 2 });
    AddSysImport("LookitModelQuick", 0, { 3, 3, 3, 2 });
    AddSysImport("LookitModelX", 0, { 3, 3, 1, 3, 2, 2, 2, 3, 2 });
    AddSysImport("LookitModelQuickX", 0, { 3, 3, 1, 3, 2, 2, 2, 3, 2 });
    
    AddSysImport("LookitNoun", 0, { 3, 3, 3, 2 });
    AddSysImport("LookitNounQuick", 0, { 3, 3, 3, 2 });
    
    AddSysImport("LookitPoint", 0, { 3, 2, 2, 2, 3, 2 });
    
    AddSysImport("LookitCameraAngle", 0, { 3, 3, 3, 2 });
    AddSysImport("LookitMouse", 0, { 3, 3, 2 });
    AddSysImport("LookitPlayer", 0, { 3, 3, 2 });
    
    AddSysImport("LookitCancel", 0, { 3 });
    
    AddSysImport("LookitLock", 0, { 3, 3, 2 });
    AddSysImport("LookitUnlock", 0, { 3, 3 });
    
    AddSysImport("SetActorLocation", 0, { 3, 3 });
    AddSysImport("SetActorOffstage", 0, { 3 });
    
    AddSysImport("SetActorPosition", 0, { 3, 3 });
    
    AddSysImport("SetEyeOffsets", 0, { 3, 2, 2, 2, 2 });
    
    AddSysImport("SetEgo", 0, { 3 });
    AddSysImport("SetEgoLocationCount", 0, { 3, 1 });
    
    AddSysImport("SetIdleGAS", 0, { 3, 3 });
    AddSysImport("SetListenGAS", 0, { 3, 3 });
    AddSysImport("SetTalkGAS", 0, { 3, 3 });
    
    AddSysImport("SetMood", 0, { 3, 3 });
    
    AddSysImport("SetNextEgo", 0, { });
    AddSysImport("SetPrevEgo", 0, { });
    
    AddSysImport("SetWalkAnim", 0, { 3, 3, 3, 3, 3 });
    
    AddSysImport("StartIdleFidget", 0, { 3 });
    AddSysImport("StartListenFidget", 0, { 3 });
    AddSysImport("StartTalkFidget", 0, { 3 });
    AddSysImport("StopFidget", 0, { 3 });
    
    AddSysImport("TurnHead", 0, { 3, 1, 1, 1 });
    AddSysImport("TurnToModel", 0, { 3, 3 });
    
    AddSysImport("WalkerBoundaryBlockModel", 0, { 3 });
    AddSysImport("WalkerBoundaryBlockRegion", 0, { 3 });
    AddSysImport("WalkerBoundaryUnblockModel", 0, { 3 });
    AddSysImport("WalkerBoundaryUnblockRegion", 0, { 3 });
    
    AddSysImport("WalkNear", 0, { 3, 3 });
    AddSysImport("WalkNearModel", 0, { 3, 3 });
    AddSysImport("WalkTo", 0, { 3, 3 });
    AddSysImport("WalkToAnimation", 0, { 3, 3 });
    AddSysImport("WalkToSeeModel", 0, { 3, 3 });
    AddSysImport("WalkToXZ", 0, { 2, 2 });
    
    AddSysImport("WasEgoEverInLocation", 1, { 3 });
    
    // ANIMATION AND DIALOGUE
    AddSysImport("AddCaptionDefault", 0, { 3 });
    AddSysImport("AddCaptionEgo", 0, { 3 });
    AddSysImport("AddCaptionVoiceOver", 0, { 3 });
    AddSysImport("ClearCaptionText", 0, { });
    
    AddSysImport("AnimEvent", 0, { 3, 3 });
    
    AddSysImport("StartDialogue", 0, { 3, 1 });
    AddSysImport("StartDialogueNoFidgets", 0, { 3, 1 });
    AddSysImport("ContinueDialogue", 0, { 1 });
    AddSysImport("ContinueDialogueNoFidgets", 0, { 1 });
    
    AddSysImport("EnableInterpolation", 0, { });
    AddSysImport("DisableInterpolation", 0, { });
    
    AddSysImport("DumpAnimator", 0, { });
    
    AddSysImport("SetConversation", 0, { 3 });
    AddSysImport("EndConversation", 0, { });
    
    AddSysImport("StartAnimation", 0, { 3 });
    AddSysImport("LoopAnimation", 0, { 3 });
    AddSysImport("StopAnimation", 0, { 3 });
    AddSysImport("StopAllAnimations", 0, { });
    
    AddSysImport("StartMorphAnimation", 0, { 3, 1, 1 });
    AddSysImport("StopMorphAnimation", 0, { 3 });
    AddSysImport("StopAllMorphAnimations", 0, { });
    
    AddSysImport("StartMom", 0, { 3 });
    AddSysImport("StartMoveAnimation", 0, { 3 });
    
    AddSysImport("StartVoiceOver", 0, { 3, 1 });
    AddSysImport("StartYak", 0, { 3 });
    
    AddSysImport("IsCurrentTime", 1, { 3 });
    */
}

// ACTORS
shpvoid Blink(std::string actorName)
{
    return 0;
}
RegFunc1(Blink, void, string);

shpvoid BlinkX(std::string actorName, std::string blinkAnim)
{
    return 0;
}
RegFunc2(BlinkX, void, string, string);

shpvoid ClearMood(std::string actorName)
{
    return 0;
}
RegFunc1(ClearMood, void, string);

shpvoid EnableEyeJitter(std::string actorName)
{
    return 0;
}
RegFunc1(EnableEyeJitter, void, string);

shpvoid DisableEyeJitter(std::string actorName)
{
    return 0;
}
RegFunc1(DisableEyeJitter, void, string);

shpvoid EyeJitter(std::string actorName)
{
    return 0;
}
RegFunc1(EyeJitter, void, string);

shpvoid DumpActorPosition(std::string actorName)
{
    return 0;
}
RegFunc1(DumpActorPosition, void, string);

shpvoid Expression(std::string actorName, std::string expression)
{
    return 0;
}
RegFunc2(Expression, void, string, string);

int GetEgoCurrentLocationCount()
{
    return 0;
}
RegFunc0(GetEgoCurrentLocationCount, int);

int GetEgoLocationCount(std::string locationName)
{
    return 0;
}
RegFunc1(GetEgoLocationCount, int, string);

std::string GetEgoName()
{
    return "";
}
RegFunc0(GetEgoName, string);

std::string GetIndexedPosition(int index)
{
    return "";
}
RegFunc1(GetIndexedPosition, string, int);

int GetPositionCount()
{
    return 0;
}
RegFunc0(GetPositionCount, int);

shpvoid Glance(std::string actorName, int percentX, int percentY, int durationMs)
{
    return 0;
}

shpvoid GlanceX(std::string actorName, int leftPercentX, int leftPercentY,
                int rightPercentX, int rightPercentY, int durationMs)
{
    return 0;
}



shpvoid InitEgoPosition(std::string positionName)
{
    GEngine::inst->GetScene()->InitEgoPosition(positionName);
    return 0;
}
RegFunc1(InitEgoPosition, void, string);

// ANIMATION AND DIALOGUE
shpvoid StartVoiceOver(std::string dialogueName, int numLines)
{
    std::string yakName = "E" + dialogueName + ".YAK";
    Yak* yak = Services::GetAssets()->LoadYak(yakName);
    yak->Play(numLines);
    return 0;
}
RegFunc2(StartVoiceOver, void, string, int);

// ENGINE
shpvoid CallSheep(std::string fileName, std::string functionName)
{
    return 0;
}
RegFunc2(CallSheep, void, string, string);

// GAME LOGIC
int IsCurrentTime(std::string time)
{
    return 0;
}
RegFunc1(IsCurrentTime, void, string);


