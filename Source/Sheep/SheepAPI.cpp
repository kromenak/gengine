//
// SheepAPI.cpp
//
// Clark Kromenaker
//
#include "SheepAPI.h"

#include <functional> // for std::hash

#include "AnimationPlayer.h"
#include "CharacterManager.h"
#include "GameProgress.h"
#include "GEngine.h"
#include "GKActor.h"
#include "Random.h"
#include "Scene.h"
#include "Services.h"
#include "SoundtrackPlayer.h"
#include "StringUtil.h"

// Required for macros to work correctly with "string" instead of "std::string".
using namespace std;

// Some defines, for readability below.
#define WAITABLE true
#define IMMEDIATE false

#define DEV_FUNC true
#define REL_FUNC false

// A big array of all our defined system functions.
// This is populated at program start and then never changed.
std::vector<SysFuncDecl> sysFuncs;

// Maps from system function name (or hash) to index in the sysFuncs vector.
// We can't map to SysFuncDecl& because vector might move memory around when populating the list.
// It IS OK to get references after initial population, since the vector never changes!
std::map<std::string, int> nameToSysFunc;
std::unordered_map<size_t, int> hashToSysFunc;

size_t CalcHashForSysFunc(const SysImport& sysImport)
{
	// This is a variation of Bernstein hash, also called djb2.
	// Original uses values 5381/33 instead of 17/31.
	// It's actually unknown why these values result in a good hash...hope it works out!
	size_t res = 17;
	std::string lowerName = StringUtil::ToLowerCopy(sysImport.name);
	res = res * 31 + std::hash<string>()(lowerName);
	res = res * 31 + std::hash<int>()((int)sysImport.argumentTypes.size());
	for(auto& argType : sysImport.argumentTypes)
	{
		res = res * 31 + std::hash<char>()(argType);
	}
	return res;
}

// A list of every defined system function. Add to this by calling AddSysFuncDecl.
// Functions are added by using RegFuncX macros, which in turn call AddSysFuncDecl and put function pointers into call maps.
void AddSysFuncDecl(const std::string& name, char retType, std::initializer_list<char> argTypes, bool waitable, bool dev)
{
	SysFuncDecl sysFunc;
	sysFunc.name = name;
	sysFunc.returnType = retType;
	for(auto argType : argTypes)
	{
		sysFunc.argumentTypes.push_back(argType);
	}
	sysFunc.waitable = waitable;
	sysFunc.devOnly = dev;
	
	sysFuncs.push_back(sysFunc);
	
	// Store a mapping from name and hash to index in the vector of system functions.
	// We can't store references because std::vector can move items around on us during population of the vector.
	nameToSysFunc[StringUtil::ToLowerCopy(name)] = (int)sysFuncs.size() - 1;
	hashToSysFunc[CalcHashForSysFunc(sysFunc)] = (int)sysFuncs.size() - 1;
}

SysFuncDecl* GetSysFuncDecl(const std::string& name)
{
	auto it = nameToSysFunc.find(name);
	if(it != nameToSysFunc.end())
	{
		return &sysFuncs[it->second];
	}
	return nullptr;
}

SysFuncDecl* GetSysFuncDecl(const SysImport* sysImport)
{
	size_t hash = CalcHashForSysFunc(*sysImport);
	auto it = hashToSysFunc.find(hash);
	if(it != hashToSysFunc.end())
	{
		return &sysFuncs[it->second];
	}
	return nullptr;
}

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
	auto it = map0.find(name);
	if(it != map0.end())
	{
		return it->second(0);
	}
	else
	{
		std::cout << "Couldn't find SysFunc0 " << name << std::endl;
		return Value(0);
	}
}

Value CallSysFunc(const std::string& name, const Value& x1)
{
	auto it = map1.find(name);
	if(it != map1.end())
	{
		return it->second(x1);
	}
	else
	{
		std::cout << "Couldn't find SysFunc1 " << name << std::endl;
		return Value(0);
	}
}

Value CallSysFunc(const std::string& name, const Value& x1, const Value& x2)
{
	auto it = map2.find(name);
	if(it != map2.end())
	{
		return it->second(x1, x2);
	}
	else
	{
		std::cout << "Couldn't find SysFunc2 " << name << std::endl;
		return Value(0);
	}
}

Value CallSysFunc(const std::string& name, const Value& x1, const Value& x2, const Value& x3)
{
	auto it = map3.find(name);
	if(it != map3.end())
	{
		return it->second(x1, x2, x3);
	}
	else
	{
		std::cout << "Couldn't find SysFunc3 " << name << std::endl;
		return Value(0);
	}
}

Value CallSysFunc(const std::string& name, const Value& x1, const Value& x2, const Value& x3,
                  const Value& x4)
{
	auto it = map4.find(name);
	if(it != map4.end())
	{
		return it->second(x1, x2, x3, x4);
	}
	else
	{
		std::cout << "Couldn't find SysFunc4 " << name << std::endl;
		return Value(0);
	}
}

Value CallSysFunc(const std::string& name, const Value& x1, const Value& x2, const Value& x3,
                  const Value& x4, const Value& x5)
{
	auto it = map5.find(name);
	if(it != map5.end())
	{
		return it->second(x1, x2, x3, x4, x5);
	}
	else
	{
		std::cout << "Couldn't find SysFunc5 " << name << std::endl;
		return Value(0);
	}
}

Value CallSysFunc(const std::string& name, const Value& x1, const Value& x2, const Value& x3,
                  const Value& x4, const Value& x5, const Value& x6)
{
	auto it = map6.find(name);
	if(it != map6.end())
	{
		return it->second(x1, x2, x3, x4, x5, x6);
	}
	else
	{
		std::cout << "Couldn't find SysFunc6 " << name << std::endl;
		return Value(0);
	}
}

// ACTORS
shpvoid Blink(std::string actorName)
{
	std::cout << "Blink " << actorName << std::endl;
    return 0;
}
RegFunc1(Blink, void, string, IMMEDIATE, REL_FUNC);

shpvoid BlinkX(std::string actorName, std::string blinkAnim)
{
	std::cout << "Blink " << actorName << " " << blinkAnim << std::endl;
    return 0;
}
RegFunc2(BlinkX, void, string, string, IMMEDIATE, REL_FUNC);

shpvoid ClearMood(std::string actorName)
{
	std::cout << "Clear mood " << actorName << std::endl;
    return 0;
}
RegFunc1(ClearMood, void, string, IMMEDIATE, REL_FUNC);

shpvoid EnableEyeJitter(std::string actorName)
{
	std::cout << "Enable eye jitter " << actorName << std::endl;
    return 0;
}
RegFunc1(EnableEyeJitter, void, string, IMMEDIATE, REL_FUNC);

shpvoid DisableEyeJitter(std::string actorName)
{
	std::cout << "Disable eye jitter " << actorName << std::endl;
    return 0;
}
RegFunc1(DisableEyeJitter, void, string, IMMEDIATE, REL_FUNC);

shpvoid EyeJitter(std::string actorName)
{
	std::cout << "Eye jitter " << actorName << std::endl;
    return 0;
}
RegFunc1(EyeJitter, void, string, IMMEDIATE, REL_FUNC);

shpvoid DumpActorPosition(std::string actorName)
{
	std::cout << "Dump actor position " << actorName << std::endl;
    return 0;
}
RegFunc1(DumpActorPosition, void, string, IMMEDIATE, DEV_FUNC);

shpvoid Expression(std::string actorName, std::string expression)
{
	std::cout << "Expression " << actorName << " " << expression << std::endl;
    return 0;
}
RegFunc2(Expression, void, string, string, IMMEDIATE, REL_FUNC);

int GetEgoCurrentLocationCount()
{
	return 0;
	/*
	// Figure out who ego is.
	GKActor* ego = GEngine::inst->GetScene()->GetEgo();
	if(ego != nullptr)
	{
		return Services::Get<GameProgress>()->GetLocationCount(ego->GetNoun());
	}
	
	// If we can't find anything...it must be at least one!?
	// Like, ego is there right now...
    return 1;
	*/
}
RegFunc0(GetEgoCurrentLocationCount, int, IMMEDIATE, REL_FUNC);

int GetEgoLocationCount(std::string locationName)
{
	//FIXME: Sheep functions can be called while the scene is being created.
	//SO...Scene is null in that case! And Ego is not yet created!
	//Need to revisit scene creation and find some stepped process to resolve this.
	Scene* scene = GEngine::inst->GetScene();
	if(scene == nullptr)
	{
		std::cout << "scene is null!" << std::endl;
		return 0;
	}
	
	// Figure out who ego is.
	GKActor* ego = scene->GetEgo();
	if(ego != nullptr)
	{
		return Services::Get<GameProgress>()->GetLocationCount(ego->GetNoun(), locationName);
	}
	
	// Can't assume 1 in this case.
    return 0;
}
RegFunc1(GetEgoLocationCount, int, string, IMMEDIATE, REL_FUNC);

std::string GetEgoName()
{
	return GEngine::inst->GetScene()->GetEgo()->GetNoun();
}
RegFunc0(GetEgoName, string, IMMEDIATE, REL_FUNC);

/*
std::string GetIndexedPosition(int index)
{
	std::cout << "GetIndexedPosition" << std::endl;
    return "";
}
RegFunc1(GetIndexedPosition, string, int, IMMEDIATE, DEV_FUNC);

int GetPositionCount()
{
	std::cout << "GetPositionCount" << std::endl;
    return 0;
}
RegFunc0(GetPositionCount, int, IMMEDIATE, DEV_FUNC);

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

shpvoid InitEgoPosition(std::string positionName)
{
    GEngine::inst->GetScene()->InitEgoPosition(positionName);
    return 0;
}
RegFunc1(InitEgoPosition, void, string, IMMEDIATE, REL_FUNC);

int IsActorAtLocation(std::string actorName, std::string locationName)
{
	std::string location = Services::Get<CharacterManager>()->GetCharacterLocation(actorName);
	return StringUtil::EqualsIgnoreCase(location, locationName) ? 1 : 0;
}
RegFunc2(IsActorAtLocation, int, string, string, IMMEDIATE, REL_FUNC);

/*
int IsActorNear(std::string actorName, std::string positionName, float distance)
{
	return 0;
}
RegFunc3(IsActorNear, int, string, string, float, IMMEDIATE, REL_FUNC);

int IsWalkingActorNear(std::string actorName, std::string positionName, float distance)
{
	return 0;
}
RegFunc3(IsWalkingActorNear, int, string, string, float, IMMEDIATE, REL_FUNC);
*/
 
int IsActorOffstage(std::string actorName)
{
	return Services::Get<CharacterManager>()->IsCharacterOffstage(actorName) ? 1 : 0;
}
RegFunc1(IsActorOffstage, int, string, IMMEDIATE, REL_FUNC);

int IsCurrentEgo(string actorName)
{
	GKActor* ego = GEngine::inst->GetScene()->GetEgo();
	if(ego == nullptr) { return 0; }
	return StringUtil::EqualsIgnoreCase(ego->GetNoun(), actorName) ? 1 : 0;
}
RegFunc1(IsCurrentEgo, int, string, IMMEDIATE, REL_FUNC);

/*
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

shpvoid SetActorLocation(string actorName, string locationName)
{
	Services::Get<CharacterManager>()->SetCharacterLocation(actorName, locationName);
	return 0;
}
RegFunc2(SetActorLocation, void, string, string, IMMEDIATE, REL_FUNC);

shpvoid SetActorOffstage(string actorName)
{
	Services::Get<CharacterManager>()->SetCharacterOffstage(actorName);
	return 0;
}
RegFunc1(SetActorOffstage, void, string, IMMEDIATE, REL_FUNC);

shpvoid SetActorPosition(std::string actorName, std::string positionName)
{
	GKActor* actor = GEngine::inst->GetScene()->GetActorByNoun(actorName);
	if(actor != nullptr)
	{
		ScenePositionData* scenePosition = GEngine::inst->GetScene()->GetPosition(positionName);
		if(scenePosition != nullptr)
		{
			// Based on docs, I don't think this sets heading...but maybe it does? Unclear.
			actor->SetPosition(scenePosition->position);
		}
	}
	return 0;
}
RegFunc2(SetActorPosition, void, string, string, IMMEDIATE, REL_FUNC);

// SetEyeOffsets

// SetEgo

// SetEgoLocationCount

/*
shpvoid SetIdleGAS(std::string actorName, std::string gasName)
{
	std::cout << "SetIdleGAS" << std::endl;
	return 0;
}
RegFunc2(SetIdleGAS, void, string, string, WAITABLE, REL_FUNC);

shpvoid SetListenGAS(std::string actorName, std::string gasName)
{
	std::cout << "SetListenGAS" << std::endl;
	return 0;
}
RegFunc2(SetListenGAS, void, string, string, WAITABLE, REL_FUNC);

shpvoid SetTalkGAS(std::string actorName, std::string gasName)
{
	std::cout << "SetTalkGAS" << std::endl;
	return 0;
}
RegFunc2(SetTalkGAS, void, string, string, WAITABLE, REL_FUNC);

shpvoid SetMood(std::string actorName, std::string moodName)
{
	std::cout << "SetMood" << std::endl;
	return 0;
}
RegFunc2(SetMood, void, string, string, IMMEDIATE, REL_FUNC);

//SetNextEgo

//SetPrevEgo

shpvoid SetWalkAnim(std::string actorName, std::string start, std::string cont,
					std::string startTurnLeft, std::string startTurnRight)
{
	std::cout << "SetWalkAnim" << std::endl;
	return 0;
}
RegFunc5(SetWalkAnim, void, string, string, string, string, string, IMMEDIATE, REL_FUNC);
*/
 
shpvoid StartIdleFidget(std::string actorName)
{
	GKActor* actor = GEngine::inst->GetScene()->GetActorByNoun(actorName);
	if(actor != nullptr)
	{
		actor->SetState(GKActor::State::Idle);
	}
	return 0;
}
RegFunc1(StartIdleFidget, void, string, WAITABLE, REL_FUNC);

shpvoid StartListenFidget(std::string actorName)
{
	GKActor* actor = GEngine::inst->GetScene()->GetActorByNoun(actorName);
	if(actor != nullptr)
	{
		actor->SetState(GKActor::State::Listen);
	}
	return 0;
}
RegFunc1(StartListenFidget, void, string, WAITABLE, REL_FUNC);

shpvoid StartTalkFidget(std::string actorName) // WAIT
{
	GKActor* actor = GEngine::inst->GetScene()->GetActorByNoun(actorName);
	if(actor != nullptr)
	{
		actor->SetState(GKActor::State::Talk);
	}
	return 0;
}
RegFunc1(StartTalkFidget, void, string, WAITABLE, REL_FUNC);

/*
shpvoid StopFidget(std::string actorName)
{
	std::cout << "StopFidget" << std::endl;
	return 0;
}
RegFunc1(StopFidget, void, string, WAITABLE, REL_FUNC);
 
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

shpvoid WalkerBoundaryBlockModel(std::string modelName)
{
	std::cout << "WalkerBoundaryBlockModel" << std::endl;
	return 0;
}
RegFunc1(WalkerBoundaryBlockModel, void, string, IMMEDIATE, REL_FUNC);

shpvoid WalkerBoundaryBlockRegion(int regionIndex, int regionBoundaryIndex)
{
	std::cout << "WalkerBoundaryBlockRegion" << std::endl;
	return 0;
}
RegFunc2(WalkerBoundaryBlockRegion, void, int, int, IMMEDIATE, REL_FUNC);

shpvoid WalkerBoundaryUnblockModel(std::string modelName)
{
	std::cout << "WalkerBoundaryUnblockModel" << std::endl;
	return 0;
}
RegFunc1(WalkerBoundaryUnblockModel, void, string, IMMEDIATE, REL_FUNC);

shpvoid WalkerBoundaryUnblockRegion(int regionIndex, int regionBoundaryIndex)
{
	std::cout << "WalkerBoundaryUnblockRegion" << std::endl;
	return 0;
}
RegFunc2(WalkerBoundaryUnblockRegion, void, int, int, IMMEDIATE, REL_FUNC);

shpvoid WalkNear(std::string actorName, std::string positionName)
{
	std::cout << "WalkNear" << std::endl;
	return 0;
}
RegFunc2(WalkNear, void, string, string, WAITABLE, REL_FUNC);

shpvoid WalkNearModel(std::string actorName, std::string modelName)
{
	std::cout << "WalkNearModel" << std::endl;
	return 0;
}
RegFunc2(WalkNearModel, void, string, string, WAITABLE, REL_FUNC);

shpvoid WalkTo(std::string actorName, std::string positionName)
{
	std::cout << "WalkTo" << std::endl;
	return 0;
}
RegFunc2(WalkTo, void, string, string, WAITABLE, REL_FUNC);

shpvoid WalkToAnimation(std::string actorName, std::string animationName)
{
	std::cout << "WalkToAnimation" << std::endl;
	return 0;
}
RegFunc2(WalkToAnimation, void, string, string, WAITABLE, REL_FUNC);

shpvoid WalkToSeeModel(std::string actorName, std::string modelName)
{
	std::cout << "WalkToSeeModel" << std::endl;
	return 0;
}
RegFunc2(WalkToSeeModel, void, string, string, WAITABLE, REL_FUNC);

//WalkToXZ
*/
 
int WasEgoEverInLocation(string locationName)
{
	// Figure out who ego is.
	GKActor* ego = GEngine::inst->GetScene()->GetEgo();
	if(ego != nullptr)
	{
		return Services::Get<GameProgress>()->GetLifetimeLocationCount(ego->GetNoun(), locationName);
	}
	
	// No ego means we assume he/she wasn't ever in the location...
	return 0;
}
RegFunc1(WasEgoEverInLocation, int, string, IMMEDIATE, REL_FUNC);

// ANIMATION AND DIALOGUE

//AddCaptionDefault

//AddCaptionEgo

//AddCaptionVoiceOver

//ClearCaptionText

//AnimEvent

/*
shpvoid StartDialogue(std::string dialogueName, int numLines)
{
	std::cout << "StartDialogue" << std::endl;
	return 0;
}
RegFunc2(StartDialogue, void, string, int, WAITABLE, REL_FUNC);

shpvoid StartDialogueNoFidgets(std::string dialogueName, int numLines)
{
	std::cout << "StartDialogueNoFidgets" << std::endl;
	return 0;
}
RegFunc2(StartDialogueNoFidgets, void, string, int, WAITABLE, REL_FUNC);

shpvoid ContinueDialogue(int numLines)
{
	std::cout << "ContinueDialogue" << std::endl;
	return 0;
}
RegFunc1(ContinueDialogue, void, int, WAITABLE, REL_FUNC);

shpvoid ContinueDialogueNoFidgets(int numLines)
{
	std::cout << "ContinueDialogueNoFidgets" << std::endl;
	return 0;
}
RegFunc1(ContinueDialogueNoFidgets, void, int, WAITABLE, REL_FUNC);
*/
 
//EnableInterpolation

//DisableInterpolation

//DumpAnimator

/*
shpvoid SetConversation(std::string conversationName)
{
	std::cout << "SetConversation" << std::endl;
	return 0;
}
RegFunc1(SetConversation, void, string, WAITABLE, REL_FUNC);

shpvoid EndConversation()
{
	std::cout << "EndConversation" << std::endl;
	return 0;
}
RegFunc0(EndConversation, void, WAITABLE, REL_FUNC);
*/
 
shpvoid StartAnimation(std::string animationName)
{
	Animation* animation = Services::GetAssets()->LoadAnimation(animationName);
	if(animation != nullptr)
	{
		SheepThread* currentThread = Services::GetSheep()->GetCurrentThread();
		GEngine::inst->GetScene()->GetAnimationPlayer()->Play(animation, currentThread->AddWait());
	}
	return 0;
}
RegFunc1(StartAnimation, void, string, WAITABLE, REL_FUNC);

/*
shpvoid StartMoveAnimation(std::string animationName)
{
	std::cout << "StartMoveAnimation" << std::endl;
	return 0;
}
RegFunc1(StartMoveAnimation, void, string, WAITABLE, REL_FUNC);

shpvoid StartMom(std::string momAnimationName)
{
	std::cout << "StartMom" << std::endl;
	return 0;
}
RegFunc1(StartMom, void, string, WAITABLE, REL_FUNC);

shpvoid LoopAnimation(std::string animationName)
{
	std::cout << "LoopAnimation" << std::endl;
	return 0;
}
RegFunc1(LoopAnimation, void, string, IMMEDIATE, REL_FUNC);

shpvoid StopAnimation(std::string animationName)
{
	std::cout << "StopAnimation" << std::endl;
	return 0;
}
RegFunc1(StopAnimation, void, string, IMMEDIATE, REL_FUNC);

//StopAllAnimations

shpvoid StartMorphAnimation(std::string animationName, int animStartFrame, int morphFrames)
{
	std::cout << "StartMorphAnimation" << std::endl;
	return 0;
}
RegFunc3(StartMorphAnimation, void, string, int, int, WAITABLE, REL_FUNC);

shpvoid StopMorphAnimation(std::string animationName)
{
	std::cout << "StopMorphAnimation" << std::endl;
	return 0;
}
RegFunc1(StopMorphAnimation, void, string, IMMEDIATE, REL_FUNC);

//StopAllMorphAnimations
*/
 
shpvoid StartVoiceOver(string dialogueName, int numLines)
{
    string yakName = "E" + dialogueName + ".YAK";
    Yak* yak = Services::GetAssets()->LoadYak(yakName);
    yak->Play(numLines);
    return 0;
}
RegFunc2(StartVoiceOver, void, string, int, WAITABLE, REL_FUNC);

//StartYak

// APPLICATION
shpvoid AddPath(std::string pathName)
{
	Services::GetAssets()->AddSearchPath(pathName);
	return 0;
}
RegFunc1(AddPath, void, string, IMMEDIATE, DEV_FUNC);

/*
shpvoid FullScanPaths()
{
	// Scans and indexes assets on all search paths.
	// Really only useful when dealing with loose files.
	return 0;
}
RegFunc0(FullScanPaths, void, IMMEDIATE, DEV_FUNC);

shpvoid RescanPaths()
{
	// Same as full scan paths, but dumps any existing indexes as well.
	return 0;
}
RegFunc0(RescanPaths, void, IMMEDIATE, DEV_FUNC);

shpvoid DumpBuildInfo()
{
	return 0;
}
RegFunc0(DumpBuildInfo, void, IMMEDIATE, DEV_FUNC);

shpvoid DumpLayerStack()
{
	return 0;
}
RegFunc0(DumpLayerStack, void, IMMEDIATE, DEV_FUNC);

shpvoid Edit(std::string filename)
{
	// Should open text editor defined in INI or default with file.
	return 0;
}
RegFunc1(Edit, void, string, IMMEDIATE, DEV_FUNC);

shpvoid Open(std::string filename)
{
	// Tells OS to open a particular file.
	return 0;
}
RegFunc1(Open, void, string, IMMEDIATE, DEV_FUNC);
*/
 
shpvoid ForceQuitGame()
{
	GEngine::inst->Quit();
	return 0;
}
RegFunc0(ForceQuitGame, void, IMMEDIATE, DEV_FUNC);

shpvoid QuitApp()
{
	//TODO: Should quit nicely by asking "are you sure" and making sure you saved.
	GEngine::inst->Quit();
	return 0;
}
RegFunc0(QuitApp, void, IMMEDIATE, DEV_FUNC);

/*
shpvoid FullReset()
{
	// Clear caches, unload everything.
	// Re-enter current scene.
	return 0;
}
RegFunc0(FullReset, void, IMMEDIATE, DEV_FUNC);

//RefreshScreen

shpvoid Screenshot()
{
	std::cout << "Screenshot" << std::endl;
	return 0;
}
RegFunc0(Screenshot, void, IMMEDIATE, REL_FUNC);

shpvoid ScreenshotX(std::string filename)
{
	return 0;
}
RegFunc1(ScreenshotX, void, string, IMMEDIATE, DEV_FUNC);

shpvoid ShowBinocs()
{
	return 0;
}
RegFunc0(ShowBinocs, void, IMMEDIATE, REL_FUNC);

shpvoid ShowDrivingInterface()
{
	return 0;
}
RegFunc0(ShowDrivingInterface, void, IMMEDIATE, REL_FUNC);

shpvoid ShowFingerprintInterface(std::string nounName)
{
	return 0;
}
RegFunc1(ShowFingerprintInterface, void, string, IMMEDIATE, REL_FUNC);

shpvoid ShowSidney()
{
	return 0;
}
RegFunc0(ShowSidney, void, IMMEDIATE, REL_FUNC);

shpvoid StartGame()
{
	// Same thing as hitting "Play" on title screen.
	return 0;
}
RegFunc0(StartGame, void, IMMEDIATE, DEV_FUNC);
*/
 
// CAMERA
/*
shpvoid CameraBoundaryBlockModel(std::string modelName)
{
	std::cout << "CameraBoundaryBlockModel" << std::endl;
	return 0;
}
RegFunc1(CameraBoundaryBlockModel, void, string, IMMEDIATE, REL_FUNC);

shpvoid CameraBoundaryUnblockModel(std::string modelName)
{
	std::cout << "CameraBoundaryUnblockModel" << std::endl;
	return 0;
}
RegFunc1(CameraBoundaryUnblockModel, void, string, IMMEDIATE, REL_FUNC);
*/
 
shpvoid CutToCameraAngle(std::string cameraName)
{
	GEngine::inst->GetScene()->SetCameraPosition(cameraName);
	return 0;
}
RegFunc1(CutToCameraAngle, void, string, IMMEDIATE, REL_FUNC);

/*
shpvoid CutToCameraAngleX(float horizAngle, float vertAngle, float x, float y, float z) // DEV
{
	return 0;
}
RegFunc5(CutToCameraAngleX, void, float, float, float, float, float, IMMEDIATE, DEV_FUNC);
*/
 
shpvoid ForceCutToCameraAngle(std::string cameraName)
{
	GEngine::inst->GetScene()->SetCameraPosition(cameraName);
	return 0;
}
RegFunc1(ForceCutToCameraAngle, void, string, IMMEDIATE, REL_FUNC);

/*
shpvoid DefaultInspect(std::string noun)
{
	std::cout << "DefaultInspect" << std::endl;
	return 0;
}
RegFunc1(DefaultInspect, void, string, WAITABLE, REL_FUNC);

//EnableCameraBoundaries
//DisableCameraBoundaries

//DumpCamera
//DumpCameraAngles

//GetCameraAngleCount
//GetIndexedCameraAngle

shpvoid SetCameraGlide(int glide)
{
	// glide is 0 or 1
	return 0;
}
RegFunc1(SetCameraGlide, void, int, IMMEDIATE, REL_FUNC);

shpvoid GlideToCameraAngle(std::string cameraName)
{
	return 0;
}
RegFunc1(GlideToCameraAngle, void, string, WAITABLE, REL_FUNC);

shpvoid InspectObject()
{
	// Glide to inspect camera position/heading.
	return 0;
}
RegFunc0(InspectObject, void, WAITABLE, REL_FUNC);

shpvoid Uninspect()
{
	// Glide back to camera position/heading before inspecting object.
	return 0;
}
RegFunc0(Uninspect, void, WAITABLE, REL_FUNC);

float GetCameraFOV()
{
	return 0.0f;
}
RegFunc0(GetCameraFOV, float, IMMEDIATE, REL_FUNC);

shpvoid SetCameraFOV()
{
	return 0;
}
RegFunc0(SetCameraFOV, void, IMMEDIATE, REL_FUNC);
*/
 
// CONSTRUCTION MODE
//ShowConstruction
//HideConstruction

//CreateCameraAngleGizmo
//CreateCameraAngleGizmoX
//ShowCameraAngleGizmo
//HideCameraAngleGizmo

//CreatePositionGizmo
//CreatePositionGizmoX
//ShowPositionGizmo
//HidePositionGizmo

//ShowAmbientMapGizmo
//HideAmbientMapGizmo

//ShowWalkerBoundaryGizmo
//HideWalkerBoundaryGizmo

//SetSceneViewport

//TextInspectCameraGizmo
//TextInspectCameraGizmoX

//TextInspectPositionGizmo
//TextInspectPositionGizmoX

//ViewportInspectCameraGizmo
//ViewportInspectCameraGizmoX

// DEBUGGING
//AddTemplate
//RemoveTemplate

//Alias
//Unalias

//BindDebugKey
//UnbindDebugKey

//OpenConsole
//CloseConsole
//ToggleConsole

//SetConsole
//InsertConsole
//ClearConsole
//ClearConsoleBuffer

//GetDebugFlag
//SetDebugFlag
//ClearDebugFlag
//ToggleDebugFlag

//DumpDebugFlags
//DumpFile
//DumpLockedObjects
//DumpMemoryUsage
//DumpPathFileMap
//DumpUsedPaths
//DumpUsedFiles

//ReportMemoryUsage
//ReportSurfaceMemoryUsage

//GetTimeMultiplier
//SetTimeMultiplier

// ENGINE
shpvoid Call(std::string functionName)
{
	SheepThread* currentThread = Services::GetSheep()->GetCurrentThread();
	if(currentThread != nullptr && currentThread->mAttachedSheep != nullptr)
	{
		SheepScript* sheep = currentThread->mAttachedSheep->mSheepScript;
		if(sheep != nullptr)
		{
			Services::GetSheep()->Execute(sheep, functionName, currentThread->AddWait());
		}
	}
	return 0;
}
RegFunc1(Call, void, string, WAITABLE, REL_FUNC);

shpvoid CallDefaultSheep(string fileName)
{
	SheepScript* script = Services::GetAssets()->LoadSheep(fileName);
	if(script != nullptr)
	{
		SheepThread* currentThread = Services::GetSheep()->GetCurrentThread();
		Services::GetSheep()->Execute(script, currentThread != nullptr ? currentThread->AddWait() : nullptr);
	}
	return 0;
}
RegFunc1(CallDefaultSheep, void, string, WAITABLE, REL_FUNC);

shpvoid CallSheep(string fileName, string functionName)
{
	// Make sure function name has the '$' suffix.
	// Some GK3 data files do this, some don't!
	if(functionName[functionName.size() - 1] != '$')
	{
		functionName.append("$");
	}
	
	SheepThread* currentThread = Services::GetSheep()->GetCurrentThread();
	Services::GetSheep()->Execute(fileName, functionName, currentThread != nullptr ? currentThread->AddWait() : nullptr);
    return 0;
}
RegFunc2(CallSheep, void, string, string, WAITABLE, REL_FUNC);

/*
shpvoid CallGlobal(std::string functionName)
{
	std::cout << "CallGlobal" << std::endl;
	return 0;
}
RegFunc1(CallGlobal, void, string, WAITABLE, REL_FUNC);

shpvoid CallGlobalSheep(std::string sheepFileName, std::string functionName)
{
	std::cout << "CallGlobalSheep" << std::endl;
	return 0;
}
RegFunc2(CallGlobalSheep, void, string, string, WAITABLE, REL_FUNC);
*/
 
//CallIndexedSheep

//EnableCinematics
//DisableCinematics

//EnableIncrementalRendering
//DisableIncrementalRendering

//EnableSheepCaching
//DisableSheepCaching
//DisableCurrentSheepCaching

//DumpActiveSheepObjects
//DumpActiveSheepThreads
//DumpCommands
//DumpRawSheep
//DumpSheepEngine

//ExecCommand
//FindCommand
//HelpCommand

std::string GetCurrentSheepFunction()
{
	SheepThread* currentThread = Services::GetSheep()->GetCurrentThread();
	if(currentThread != nullptr)
	{
		return currentThread->mFunctionName;
	}
	return "";
}
RegFunc0(GetCurrentSheepFunction, string, IMMEDIATE, REL_FUNC);

std::string GetCurrentSheepName()
{
	SheepThread* currentThread = Services::GetSheep()->GetCurrentThread();
	if(currentThread != nullptr && currentThread->mAttachedSheep != nullptr)
	{
		// Specifically, return name with no extension.
		return currentThread->mAttachedSheep->mSheepScript->GetNameNoExtension();
	}
	return "";
}
RegFunc0(GetCurrentSheepName, string, IMMEDIATE, REL_FUNC);

/*
shpvoid SetGlobalSheep()
{
	// Attach current sheep to global layer.
	// Means sheep stays active even when adding new game layers or changing scenes.
	std::cout << "SetGlobalSheep" << std::endl;
	return 0;
}
RegFunc0(SetGlobalSheep, void, IMMEDIATE, REL_FUNC);

shpvoid SetTopSheep()
{
	// Attach current sheep to top layer.
	// Essentially, undoes a SetGlobalSheep.
	// Sheep will stop on scene change.
	std::cout << "SetTopSheep" << std::endl;
	return 0;
}
RegFunc0(SetTopSheep, void, IMMEDIATE, REL_FUNC);
*/
 
//NukeAllSheep
//NukeSheep

//GetGamma
//SetGamma

//SaveSprite
//SaveTexture
//SaveTextureX

//SetRenderFlat
//SetRenderFull
//SetRenderShaded
//SetRenderWireframe

//SetShadowTypeBlobby
//SetShadowTypeModel
//SetShadowTypeNone

//SetSurfaceHigh
//SetSurfaceLow
//SetSurfaceNormal

/*
shpvoid SetTimerMs(int milliseconds)
{
	// Should throw error if not waited upon!
	return 0;
}
RegFunc1(SetTimerMs, void, int, WAITABLE, REL_FUNC);

shpvoid SetTimerSeconds(float seconds)
{
	// Should throw error if not waited upon!
	return 0;
}
RegFunc1(SetTimerSeconds, void, float, WAITABLE, REL_FUNC);
*/
 
//ThrowException

//UnloadAll
//UnloadAllAnimations
//UnloadAllModels
//UnloadAllMovies
//UnloadAllScenes
//UnloadAllSounds
//UnloadAllSprites
//UnloadAllTextures

//UnloadAnimation
//UnloadModel
//UnloadMovie
//UnloadScene
//UnloadSound
//UnloadSprite
//UnloadTexture

// GAME LOGIC
//AddCaseLogic
//CheckCase
//CommitCaseLogic
//DumpCaseLogic
//ResetCaseLogic

int GetFlag(std::string flagName)
{
	return Services::Get<GameProgress>()->GetFlag(flagName);
}
RegFunc1(GetFlag, int, string, IMMEDIATE, REL_FUNC);

/*
int GetFlagInt(int flagEnum)
{
	// This function gets a flag, not by name, but by internal integer value.
	// I'm a bit unclear how any Sheep caller would know this value, and how
	// this value would be persistant across runs of the game...
	std::cout << "GetFlagInt was called!" << std::endl;
	return 0;
}
RegFunc1(GetFlagInt, int, int, IMMEDIATE, REL_FUNC);
*/
 
shpvoid SetFlag(std::string flagName)
{
	Services::Get<GameProgress>()->SetFlag(flagName);
	return 0;
}
RegFunc1(SetFlag, void, string, IMMEDIATE, REL_FUNC);

shpvoid ClearFlag(std::string flagName)
{
	Services::Get<GameProgress>()->ClearFlag(flagName);
	return 0;
}
RegFunc1(ClearFlag, void, string, IMMEDIATE, REL_FUNC);

/*
shpvoid DumpFlags()
{
	return 0;
}
RegFunc0(DumpFlags, void, IMMEDIATE, DEV_FUNC);
*/
 
//DumpNouns

int GetChatCount(std::string noun)
{
	return Services::Get<GameProgress>()->GetChatCount(noun);
}
RegFunc1(GetChatCount, int, string, IMMEDIATE, REL_FUNC);

/*
int GetChatCountInt(int nounEnum)
{
	//TODO: Should be able to call this from sheep using $n variable.
	std::cout << "GetChatCountInt" << std::endl;
	return 0;
}
RegFunc1(GetChatCountInt, int, int, IMMEDIATE, REL_FUNC);
*/
 
shpvoid SetChatCount(std::string noun, int count)
{
	Services::Get<GameProgress>()->SetChatCount(noun, count);
	return 0;
}
RegFunc2(SetChatCount, void, string, int, IMMEDIATE, DEV_FUNC);

int GetGameVariableInt(std::string varName)
{
	return Services::Get<GameProgress>()->GetGameVariable(varName);
}
RegFunc1(GetGameVariableInt, int, string, IMMEDIATE, REL_FUNC);

shpvoid IncGameVariableInt(std::string varName)
{
	Services::Get<GameProgress>()->IncGameVariable(varName);
	return 0;
}
RegFunc1(IncGameVariableInt, void, string, IMMEDIATE, REL_FUNC);

shpvoid SetGameVariableInt(std::string varName, int value)
{
	Services::Get<GameProgress>()->SetGameVariable(varName, value);
	return 0;
}
RegFunc2(SetGameVariableInt, void, string, int, IMMEDIATE, REL_FUNC);

int GetNounVerbCount(std::string noun, std::string verb)
{
	return Services::Get<GameProgress>()->GetNounVerbCount(noun, verb);
}
RegFunc2(GetNounVerbCount, int, string, string, IMMEDIATE, REL_FUNC);

/*
int GetNounVerbCountInt(int nounEnum, int verbEnum)
{
	//TODO: Should be able to call this from sheep using $n and $v variables.
	std::cout << "GetNounVerbCountInt" << std::endl;
	return 0;
}
RegFunc2(GetNounVerbCountInt, int, int, int, IMMEDIATE, REL_FUNC);
*/
 
shpvoid IncNounVerbCount(string noun, string verb)
{
	Services::Get<GameProgress>()->IncNounVerbCount(noun, verb);
	return 0;
}
RegFunc2(IncNounVerbCount, void, string, string, IMMEDIATE, REL_FUNC);

shpvoid SetNounVerbCount(string noun, string verb, int count)
{
	Services::Get<GameProgress>()->SetNounVerbCount(noun, verb, count);
	return 0;
}
RegFunc3(SetNounVerbCount, void, string, string, int, IMMEDIATE, REL_FUNC);

/*
shpvoid SetNounVerbCountBoth(string noun, string verb, int count)
{
	//TODO: This function is undocumented, so I'm not sure how it differs from SetNounVerbCount.
	//TODO: It has same return type and arguments. Maybe we track noun/verb counts separately for Gabe/Grace?
	return SetNounVerbCount(noun, verb, count);
}
RegFunc3(SetNounVerbCountBoth, void, string, string, int, IMMEDIATE, REL_FUNC);

shpvoid TriggerNounVerb(string noun, string verb)
{
	return 0;
}
RegFunc2(TriggerNounVerb, void, string, string, IMMEDIATE, DEV_FUNC);
*/
 
int GetScore()
{
	return Services::Get<GameProgress>()->GetScore();
}
RegFunc0(GetScore, int, IMMEDIATE, REL_FUNC);

shpvoid IncreaseScore(int value)
{
	Services::Get<GameProgress>()->IncreaseScore(value);
	return 0;
}
RegFunc1(IncreaseScore, void, int, IMMEDIATE, REL_FUNC);

shpvoid SetScore(int score)
{
	Services::Get<GameProgress>()->SetScore(score);
	return 0;
}
RegFunc1(SetScore, void, int, IMMEDIATE, DEV_FUNC);

/*
int GetTopicCount(std::string noun, std::string verb)
{
	return 0;
}
RegFunc2(GetTopicCount, int, string, string, IMMEDIATE, REL_FUNC);

int GetTopicCountInt(int nounEnum, int verbEnum)
{
	return 0;
}
RegFunc2(GetTopicCountInt, int, int, int, IMMEDIATE, REL_FUNC);

int HasTopicsLeft(std::string noun)
{
	return 0;
}
RegFunc1(HasTopicsLeft, int, string, IMMEDIATE, REL_FUNC);

//SetTopicCount
*/
 
int IsCurrentLocation(std::string location)
{
	std::string currentLocation = Services::Get<GameProgress>()->GetLocation();
	return StringUtil::EqualsIgnoreCase(currentLocation, location);
}
RegFunc1(IsCurrentLocation, int, string, IMMEDIATE, REL_FUNC);

int IsCurrentTime(std::string timeCode)
{
	std::string currentTimeCode = Services::Get<GameProgress>()->GetTimeCode();
    return StringUtil::EqualsIgnoreCase(currentTimeCode, timeCode);
}
RegFunc1(IsCurrentTime, int, string, IMMEDIATE, REL_FUNC);

int WasLastLocation(std::string location)
{
	std::string lastLocation = Services::Get<GameProgress>()->GetLastLocation();
	return StringUtil::EqualsIgnoreCase(lastLocation, location);
}
RegFunc1(WasLastLocation, int, string, IMMEDIATE, REL_FUNC);

int WasLastTime(std::string timeCode)
{
	std::string lastTimeCode = Services::Get<GameProgress>()->GetLastTimeCode();
	return StringUtil::EqualsIgnoreCase(lastTimeCode, timeCode);
}
RegFunc1(WasLastTime, int, string, IMMEDIATE, REL_FUNC);

//ResetGameData

/*
shpvoid SetGameTimer(std::string noun, std::string verb, int milliseconds)
{
	std::cout << "SetGameTimer" << std::endl;
	return 0;
}
RegFunc3(SetGameTimer, void, string, string, int, IMMEDIATE, REL_FUNC);
*/
 
// GENERAL
//DrawFilledRect

shpvoid FollowOnDrivingMap(int followState)
{
	std::cout << "FollowOnDrivingMap" << std::endl;
	return 0;
}
RegFunc1(FollowOnDrivingMap, void, int, WAITABLE, REL_FUNC);

float GetRandomFloat(float lower, float upper)
{
	return Random::Range(lower, upper);
}
RegFunc2(GetRandomFloat, float, float, float, IMMEDIATE, REL_FUNC);

int GetRandomInt(int lower, int upper)
{
	// Both upper and lower are inclusive, according to docs.
	// So, add one to upper since our range function is upper-exclusive.
	return Random::Range(lower, upper + 1);
}

/*
shpvoid PlayFullScreenMovie(std::string movieName)
{
	std::cout << "PlayFullScreenMovie" << std::endl;
	return 0;
}
RegFunc1(PlayFullScreenMovie, void, string, WAITABLE, REL_FUNC);

shpvoid PlayMovie(std::string movieName)
{
	std::cout << "PlayMovie" << std::endl;
	return 0;
}
RegFunc1(PlayMovie, void, string, WAITABLE, REL_FUNC);

shpvoid SetPamphletPage(int page)
{
	std::cout << "SetPamphletPage" << std::endl;
	return 0;
}
RegFunc1(SetPamphletPage, void, int, WAITABLE, REL_FUNC);
*/
 
// INSETS
//DumpInsetNames

shpvoid ShowInset(std::string insetName)
{
	std::cout << "ShowInset" << std::endl;
	return 0;
}
RegFunc1(ShowInset, void, string, IMMEDIATE, REL_FUNC);

shpvoid HideInset()
{
	std::cout << "HideInset" << std::endl;
	return 0;
}
RegFunc0(HideInset, void, IMMEDIATE, REL_FUNC);

shpvoid ShowPlate(std::string plateName)
{
	std::cout << "ShowPlate" << std::endl;
	return 0;
}
RegFunc1(ShowPlate, void, string, IMMEDIATE, REL_FUNC);

shpvoid HidePlate(std::string plateName)
{
	std::cout << "HidePlate" << std::endl;
	return 0;
}
RegFunc1(HidePlate, void, string, IMMEDIATE, REL_FUNC);

// INVENTORY
/*
shpvoid CombineInvItems(std::string firstItemName, std::string secondItemName,
						std::string combinedItemName)
{
	std::cout << "CombineInvItems" << std::endl;
	return 0;
}
RegFunc3(CombineInvItems, void, string, string, string, IMMEDIATE, REL_FUNC);

int DoesEgoHaveInvItem(std::string itemName)
{
	std::cout << "DoesEgoHaveInvItem" << std::endl;
	return 0;
}
RegFunc1(DoesEgoHaveInvItem, int, string, IMMEDIATE, REL_FUNC);

int DoesGabeHaveInvItem(std::string itemName)
{
	std::cout << "DoesGabeHaveInvItem" << std::endl;
	return 0;
}
RegFunc1(DoesGabeHaveInvItem, int, string, IMMEDIATE, REL_FUNC);

int DoesGraceHaveInvItem(std::string itemName)
{
	std::cout << "DoesGraceHaveInvItem" << std::endl;
    return 0;
}
RegFunc1(DoesGraceHaveInvItem, int, string, IMMEDIATE, REL_FUNC);

shpvoid EgoTakeInvItem(std::string itemName)
{
	std::cout << "EgoTakeInvItem" << std::endl;
	return 0;
}
RegFunc1(EgoTakeInvItem, void, string, IMMEDIATE, REL_FUNC);

//DumpEgoActiveInvItem

shpvoid SetEgoActiveInvItem(std::string itemName)
{
	std::cout << "SetEgoActiveInvItem" << std::endl;
	return 0;
}
RegFunc1(SetEgoActiveInvItem, void, string, IMMEDIATE, REL_FUNC);

shpvoid ShowInventory()
{
	std::cout << "ShowInventory" << std::endl;
	return 0;
}
RegFunc0(ShowInventory, void, IMMEDIATE, REL_FUNC);

shpvoid HideInventory()
{
	std::cout << "HideInventory" << std::endl;
	return 0;
}
RegFunc0(HideInventory, void, IMMEDIATE, REL_FUNC);

shpvoid InventoryInspect(std::string itemName)
{
	std::cout << "InventoryInspect" << std::endl;
	return 0;
}
RegFunc1(InventoryInspect, void, string, IMMEDIATE, REL_FUNC);

shpvoid InventoryUninspect()
{
	std::cout << "InventoryUninspect" << std::endl;
	return 0;
}
RegFunc0(InventoryUninspect, void, IMMEDIATE, REL_FUNC);

shpvoid SetInvItemStatus(std::string itemName, std::string status)
{
	std::cout << "SetInvItemStatus" << std::endl;
	return 0;
}
RegFunc2(SetInvItemStatus, void, string, string, IMMEDIATE, REL_FUNC);
*/
 
// MODELS
/*
shpvoid SetModelShadowTexture(std::string modelName, std::string textureName)
{
	std::cout << "SetModelShadowTexture" << std::endl;
	return 0;
}

shpvoid ClearModelShadowTexture(std::string modelName)
{
	std::cout << "ClearModelShadowTexture" << std::endl;
	return 0;
}

shpvoid SetPropGas(std::string modelName, std::string gasName)
{
	std::cout << "SetPropGas" << std::endl;
	return 0;
}

shpvoid ClearPropGas(std::string modelName)
{
	std::cout << "ClearPropGas" << std::endl;
	return 0;
}
*/

int DoesModelExist(std::string modelName)
{
	GKActor* actor = GEngine::inst->GetScene()->GetActorByModelName(modelName);
	return actor != nullptr ? 1 : 0;
}

int DoesSceneModelExist(std::string modelName)
{
	return GEngine::inst->GetScene()->DoesSceneModelExist(modelName) ? 1 : 0;
}

//DumpModel
//DumpModelNames
//DumpSceneModelNames

shpvoid ShowModel(std::string modelName)
{
	GKActor* actor = GEngine::inst->GetScene()->GetActorByModelName(modelName);
	if(actor != nullptr)
	{
		actor->Actor::SetState(Actor::State::Enabled);
	}
	return 0;
}
RegFunc1(ShowModel, void, string, IMMEDIATE, REL_FUNC);

shpvoid HideModel(std::string modelName)
{
	GKActor* actor = GEngine::inst->GetScene()->GetActorByModelName(modelName);
	if(actor != nullptr)
	{
		actor->Actor::SetState(Actor::State::Disabled);
	}
	return 0;
}
RegFunc1(HideModel, void, string, IMMEDIATE, REL_FUNC);

/*
shpvoid ShowModelGroup(std::string groupName)
{
	std::cout << "ShowModelGroup" << std::endl;
	return 0;
}
RegFunc1(ShowModelGroup, void, string, IMMEDIATE, REL_FUNC);

shpvoid HideModelGroup(std::string groupName)
{
	std::cout << "HideModelGroup" << std::endl;
	return 0;
}
RegFunc1(HideModelGroup, void, string, IMMEDIATE, REL_FUNC);
*/
 
shpvoid ShowSceneModel(std::string modelName)
{
	GEngine::inst->GetScene()->SetSceneModelVisibility(modelName, true);
	return 0;
}
RegFunc1(ShowSceneModel, void, string, IMMEDIATE, REL_FUNC);

shpvoid HideSceneModel(std::string modelName)
{
	GEngine::inst->GetScene()->SetSceneModelVisibility(modelName, false);
	return 0;
}
RegFunc1(HideSceneModel, void, string, IMMEDIATE, REL_FUNC);

int IsModelVisible(std::string modelName)
{
	GKActor* actor = GEngine::inst->GetScene()->GetActorByModelName(modelName);
	if(actor != nullptr)
	{
		return actor->GetState() == Actor::State::Enabled ? 1 : 0;
	}
	return 0;
}
RegFunc1(IsModelVisible, int, string, IMMEDIATE, REL_FUNC);

int IsSceneModelVisible(std::string modelName)
{
	return GEngine::inst->GetScene()->IsSceneModelVisible(modelName) ? 1 : 0;
}
RegFunc1(IsSceneModelVisible, int, string, IMMEDIATE, REL_FUNC);

/*
shpvoid StartPropFidget(std::string modelName)
{
	std::cout << "StartPropFidget" << std::endl;
	return 0;
}
RegFunc1(StartPropFidget, void, string, IMMEDIATE, REL_FUNC);

shpvoid StopPropFidget(std::string modelName)
{
	std::cout << "StopPropFidget" << std::endl;
	return 0;
}
RegFunc1(StopPropFidget, void, string, IMMEDIATE, REL_FUNC);
*/
 
// REPORTS
shpvoid AddStreamContent(std::string streamName, std::string content)
{
	if(StringUtil::EqualsIgnoreCase(content, "begin"))
	{
		
	}
	else if(StringUtil::EqualsIgnoreCase(content, "content"))
	{
		
	}
	else if(StringUtil::EqualsIgnoreCase(content, "end"))
	{
		
	}
	else if(StringUtil::EqualsIgnoreCase(content, "category"))
	{
		
	}
	else if(StringUtil::EqualsIgnoreCase(content, "date"))
	{
		
	}
	else if(StringUtil::EqualsIgnoreCase(content, "time"))
	{
		
	}
	else if(StringUtil::EqualsIgnoreCase(content, "debug"))
	{
		
	}
	else if(StringUtil::EqualsIgnoreCase(content, "timeblock"))
	{
		
	}
	else if(StringUtil::EqualsIgnoreCase(content, "location"))
	{
		
	}
	else if(StringUtil::EqualsIgnoreCase(content, "machine"))
	{
		
	}
	else if(StringUtil::EqualsIgnoreCase(content, "user"))
	{
		
	}
	else if(StringUtil::EqualsIgnoreCase(content, "all"))
	{
		
	}
	else
	{
		//ERROR: Unknown content!
	}
	
	//Services::GetReports()->AddStreamContent(streamName, content);
	return 0;
}
RegFunc2(AddStreamContent, void, string, string, IMMEDIATE, DEV_FUNC);

shpvoid ClearStreamContent(std::string streamName)
{
	Services::GetReports()->ClearStreamContent(streamName);
	return 0;
}
RegFunc1(ClearStreamContent, void, string, IMMEDIATE, DEV_FUNC);

shpvoid RemoveStreamContent(std::string streamName, std::string content)
{
	return 0;
}
RegFunc2(RemoveStreamContent, void, string, string, IMMEDIATE, DEV_FUNC);

shpvoid AddStreamOutput(std::string streamName, std::string output)
{
	return 0;
}
RegFunc2(AddStreamOutput, void, string, string, IMMEDIATE, DEV_FUNC);

shpvoid ClearStreamOutput(std::string streamName)
{
	Services::GetReports()->ClearStreamOutput(streamName);
	return 0;
}
RegFunc1(ClearStreamOutput, void, string, IMMEDIATE, DEV_FUNC);

shpvoid RemoveStreamOutput(std::string streamName, std::string output)
{
	return 0;
}
RegFunc2(RemoveStreamOutput, void, string, string, IMMEDIATE, DEV_FUNC);

shpvoid DisableStream(std::string streamName)
{
	Services::GetReports()->DisableStream(streamName);
	return 0;
}
RegFunc1(DisableStream, void, string, IMMEDIATE, DEV_FUNC);

shpvoid EnableStream(std::string streamName)
{
	Services::GetReports()->EnableStream(streamName);
	return 0;
}
RegFunc1(EnableStream, void, string, IMMEDIATE, DEV_FUNC);

shpvoid HideReportGraph(std::string graphType)
{
	return 0;
}
RegFunc1(HideReportGraph, void, string, IMMEDIATE, DEV_FUNC);

shpvoid ShowReportGraph(std::string graphType)
{
	return 0;
}
RegFunc1(ShowReportGraph, void, string, IMMEDIATE, DEV_FUNC);

shpvoid SetStreamAction(std::string streamName, std::string action)
{
	return 0;
}
RegFunc2(SetStreamAction, void, string, string, IMMEDIATE, DEV_FUNC);

shpvoid SetStreamFilename(std::string streamName, std::string filename)
{
	Services::GetReports()->SetStreamFilename(streamName, filename);
	return 0;
}
RegFunc2(SetStreamFilename, void, string, string, IMMEDIATE, DEV_FUNC);

shpvoid SetStreamFileTruncate(std::string streamName, int truncate)
{
	Services::GetReports()->SetStreamFileTruncate(streamName, truncate != 0);
	return 0;
}
RegFunc2(SetStreamFileTruncate, void, string, int, IMMEDIATE, DEV_FUNC);

// SCENE
/*
shpvoid CallSceneFunction(std::string parameter)
{
	std::cout << "CallSceneFunction" << std::endl;
	return 0;
}
RegFunc1(CallSceneFunction, void, string, WAITABLE, REL_FUNC);
*/
 
//DumpLocations
//DumpPosition
//DumpPositions
//DumpTimes

//ReEnter

shpvoid SetLocation(std::string location)
{
	GEngine::inst->LoadScene(location);
	return 0;
}
RegFunc1(SetLocation, void, string, WAITABLE, REL_FUNC);

shpvoid SetLocationTime(std::string location, std::string time)
{
	Services::Get<GameProgress>()->SetTimeCode(time);
	GEngine::inst->LoadScene(location);
	return 0;
}
RegFunc2(SetLocationTime, void, string, string, WAITABLE, REL_FUNC);

shpvoid SetTime(std::string time)
{
	// Change time, but load in to the same scene we are currently in.
	Services::Get<GameProgress>()->SetTimeCode(time);
	GEngine::inst->LoadScene(Services::Get<GameProgress>()->GetLocation());
	return 0;
}
RegFunc1(SetTime, void, string, WAITABLE, REL_FUNC);

/*
shpvoid SetScene(std::string sceneName)
{
	// Loads a new SCN file. I wonder how often this happens?
	std::cout << "SetScene" << std::endl;
	return 0;
}
RegFunc1(SetScene, void, string, WAITABLE, REL_FUNC);

shpvoid Warp(std::string locationAndTime)
{
	// Lots of input handling to figure out string and handle errors...
	// Wonder if this is ever used?
	std::cout << "Warp" << std::endl;
	return 0;
}
RegFunc1(Warp, void, string, WAITABLE, REL_FUNC);
*/

// SOUND
//EnableSound
//DisableSound

//GetVolume
//SetVolume

shpvoid PlaySound(std::string soundName)
{
	Audio* audio = Services::GetAssets()->LoadAudio(soundName);
	if(audio != nullptr)
	{
		Services::GetAudio()->Play(audio);
	}
	return 0;
}
RegFunc1(PlaySound, void, string, WAITABLE, REL_FUNC);

/*
shpvoid StopSound(std::string soundName)
{
	std::cout << "StopSound" << std::endl;
	return 0;
}
RegFunc1(StopSound, void, string, IMMEDIATE, REL_FUNC);

shpvoid StopAllSounds()
{
	std::cout << "StopAllSound" << std::endl;
	return 0;
}
RegFunc0(StopAllSounds, void, IMMEDIATE, REL_FUNC);
*/
 
shpvoid PlaySoundTrack(std::string soundtrackName)
{
	Soundtrack* soundtrack = Services::GetAssets()->LoadSoundtrack(soundtrackName);
	if(soundtrack != nullptr)
	{
		GEngine::inst->GetScene()->GetSoundtrackPlayer()->Play(soundtrack);
	}
	return 0;
}
RegFunc1(PlaySoundTrack, void, string, WAITABLE, REL_FUNC);

/*
shpvoid StopSoundTrack(std::string soundtrackName)
{
	std::cout << "StopSoundTrack" << std::endl;
	return 0;
}
RegFunc1(StopSoundTrack, void, string, IMMEDIATE, REL_FUNC);

shpvoid StopAllSoundTracks()
{
	std::cout << "StopAllSoundTracks" << std::endl;
	return 0;
}
RegFunc0(StopAllSoundTracks, void, IMMEDIATE, REL_FUNC);
*/
 
// TRACING
//PrintFloat
//PrintFloatX

//PrintInt
//PrintIntX

//PrintIntHex
//PrintIntHexX

//PrintString
//PrintStringX
