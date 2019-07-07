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
#include "Scene.h"
#include "Services.h"
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
shpvoid Blink(string actorName)
{
    return 0;
}
RegFunc1(Blink, void, string, IMMEDIATE, REL_FUNC);

shpvoid BlinkX(string actorName, string blinkAnim)
{
    return 0;
}
RegFunc2(BlinkX, void, string, string, IMMEDIATE, REL_FUNC);

shpvoid ClearMood(string actorName)
{
    return 0;
}
RegFunc1(ClearMood, void, string, IMMEDIATE, REL_FUNC);

shpvoid EnableEyeJitter(string actorName)
{
    return 0;
}
RegFunc1(EnableEyeJitter, void, string, IMMEDIATE, REL_FUNC);

shpvoid DisableEyeJitter(string actorName)
{
    return 0;
}
RegFunc1(DisableEyeJitter, void, string, IMMEDIATE, REL_FUNC);

shpvoid EyeJitter(string actorName)
{
    return 0;
}
RegFunc1(EyeJitter, void, string, IMMEDIATE, REL_FUNC);

shpvoid DumpActorPosition(string actorName)
{
    return 0;
}
RegFunc1(DumpActorPosition, void, string, IMMEDIATE, DEV_FUNC);

shpvoid Expression(string actorName, string expression)
{
    return 0;
}
RegFunc2(Expression, void, string, string, IMMEDIATE, REL_FUNC);

int GetEgoCurrentLocationCount()
{
    return 1;
}
RegFunc0(GetEgoCurrentLocationCount, int, IMMEDIATE, REL_FUNC);

int GetEgoLocationCount(string locationName)
{
    return 0;
}
RegFunc1(GetEgoLocationCount, int, string, IMMEDIATE, REL_FUNC);

string GetEgoName()
{
	return GEngine::inst->GetScene()->GetEgo()->GetNoun();
}
RegFunc0(GetEgoName, string, IMMEDIATE, REL_FUNC);

std::string GetIndexedPosition(int index)
{
    return "";
}
RegFunc1(GetIndexedPosition, string, int, IMMEDIATE, DEV_FUNC);

int GetPositionCount()
{
    return 0;
}
RegFunc0(GetPositionCount, int, IMMEDIATE, DEV_FUNC);

shpvoid Glance(string actorName, int percentX, int percentY, int durationMs)
{
    return 0;
}

shpvoid GlanceX(string actorName, int leftPercentX, int leftPercentY,
                int rightPercentX, int rightPercentY, int durationMs)
{
    return 0;
}

shpvoid InitEgoPosition(string positionName)
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

int IsActorOffstage(string actorName)
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

shpvoid StartIdleFidget(string actorName) // WAIT
{
	std::cout << "Start idle fidget for " << actorName << std::endl;
	GKActor* actor = GEngine::inst->GetScene()->GetActorByNoun(actorName);
	if(actor != nullptr)
	{
		actor->SetState(GKActor::State::Idle);
	}
	return 0;
}
RegFunc1(StartIdleFidget, void, string, WAITABLE, REL_FUNC);

shpvoid StartListenFidget(string actorName) // WAIT
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

shpvoid StopFidget(std::string actorName)
{
	return 0;
}
RegFunc1(StopFidget, void, string, WAITABLE, REL_FUNC);

shpvoid TurnHead(std::string actorName, int percentX, int percentY, int durationMs)
{
	return 0;
}
RegFunc4(TurnHead, void, string, int, int, int, WAITABLE, REL_FUNC);

shpvoid TurnToModel(std::string actorName, std::string modelName)
{
	return 0;
}
RegFunc2(TurnToModel, void, string, string, WAITABLE, REL_FUNC);

shpvoid WalkerBoundaryBlockModel(std::string modelName)
{
	return 0;
}
RegFunc1(WalkerBoundaryBlockModel, void, string, IMMEDIATE, REL_FUNC);

shpvoid WalkerBoundaryBlockRegion(int regionIndex, int regionBoundaryIndex)
{
	return 0;
}
RegFunc2(WalkerBoundaryBlockRegion, void, int, int, IMMEDIATE, REL_FUNC);

shpvoid WalkerBoundaryUnblockModel(std::string modelName)
{
	return 0;
}
RegFunc1(WalkerBoundaryUnblockModel, void, string, IMMEDIATE, REL_FUNC);

shpvoid WalkerBoundaryUnblockRegion(int regionIndex, int regionBoundaryIndex)
{
	return 0;
}
RegFunc2(WalkerBoundaryUnblockRegion, void, int, int, IMMEDIATE, REL_FUNC);

int WasEgoEverInLocation(string locationName)
{
	return 0;
}
RegFunc1(WasEgoEverInLocation, int, string, IMMEDIATE, REL_FUNC);

// ANIMATION AND DIALOGUE
shpvoid StartAnimation(string animationName) // WAIT
{
	std::cout << "Start animation " << animationName << std::endl;
	Animation* animation = Services::GetAssets()->LoadAnimation(animationName);
	if(animation != nullptr)
	{
		SheepThread* currentThread = Services::GetSheep()->GetCurrentThread();
		GEngine::inst->GetScene()->GetAnimationPlayer()->Play(animation, currentThread->AddWait());
	}
	return 0;
}
RegFunc1(StartAnimation, void, string, WAITABLE, REL_FUNC);

shpvoid StartVoiceOver(string dialogueName, int numLines) // WAIT
{
    string yakName = "E" + dialogueName + ".YAK";
    Yak* yak = Services::GetAssets()->LoadYak(yakName);
    yak->Play(numLines);
    return 0;
}
RegFunc2(StartVoiceOver, void, string, int, WAITABLE, REL_FUNC);

// APPLICATION
shpvoid AddPath(std::string pathName)
{
	return 0;
}
RegFunc1(AddPath, void, string, IMMEDIATE, DEV_FUNC);

shpvoid FullScanPaths()
{
	return 0;
}
RegFunc0(FullScanPaths, void, IMMEDIATE, DEV_FUNC);

shpvoid RescanPaths()
{
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
	return 0;
}
RegFunc1(Edit, void, string, IMMEDIATE, DEV_FUNC);

shpvoid Open(std::string filename)
{
	return 0;
}
RegFunc1(Open, void, string, IMMEDIATE, DEV_FUNC);

shpvoid ForceQuitGame()
{
	return 0;
}
RegFunc0(ForceQuitGame, void, IMMEDIATE, DEV_FUNC);

shpvoid FullReset()
{
	return 0;
}
RegFunc0(FullReset, void, IMMEDIATE, DEV_FUNC);

shpvoid QuitApp()
{
	return 0;
}
RegFunc0(QuitApp, void, IMMEDIATE, DEV_FUNC);

shpvoid Screenshot()
{
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
	return 0;
}
RegFunc0(StartGame, void, IMMEDIATE, DEV_FUNC);

// CAMERA
shpvoid CutToCameraAngle(string cameraName)
{
	GEngine::inst->GetScene()->SetCameraPosition(cameraName);
	return 0;
}
RegFunc1(CutToCameraAngle, void, string, IMMEDIATE, REL_FUNC);

shpvoid CutToCameraAngleX(float horizAngle, float vertAngle, float x, float y, float z) // DEV
{
	//TODO
	return 0;
}
RegFunc5(CutToCameraAngleX, void, float, float, float, float, float, IMMEDIATE, DEV_FUNC);

// CONSTRUCTION MODE

// DEBUGGING

// ENGINE
shpvoid Call(string functionName) // WAIT
{
	//Services::GetSheep()->Execute(functionName);
	return 0;
}
RegFunc1(Call, void, string, WAITABLE, REL_FUNC);

shpvoid CallDefaultSheep(string fileName) // WAIT
{
	SheepScript* script = Services::GetAssets()->LoadSheep(fileName);
	if(script != nullptr)
	{
		Services::GetSheep()->Execute(script);
	}
	return 0;
}
RegFunc1(CallDefaultSheep, void, string, WAITABLE, REL_FUNC);

shpvoid CallSheep(string fileName, string functionName) // WAIT
{
	// Make sure function name has the '$' suffix.
	// Some GK3 data files do this, some don't!
	if(functionName[functionName.size() - 1] != '$')
	{
		functionName.append("$");
	}
	
	std::cout << "CallSheep " << fileName << ", " << functionName << std::endl;
	SheepThread* currentThread = Services::GetSheep()->GetCurrentThread();
	Services::GetSheep()->Execute(fileName, functionName, currentThread->AddWait());
    return 0;
}
RegFunc2(CallSheep, void, string, string, WAITABLE, REL_FUNC);

// GAME LOGIC
shpvoid ClearFlag(string flagName)
{
	Services::Get<GameProgress>()->ClearFlag(flagName);
	return 0;
}
RegFunc1(ClearFlag, void, string, IMMEDIATE, REL_FUNC);

shpvoid DumpFlags() // DEV
{
	//TODO
	return 0;
}
RegFunc0(DumpFlags, void, IMMEDIATE, DEV_FUNC);

int GetFlag(string flagName)
{
	return Services::Get<GameProgress>()->GetFlag(flagName);
}
RegFunc1(GetFlag, int, string, IMMEDIATE, REL_FUNC);

int GetFlagInt(int flagEnum)
{
	// This function gets a flag, not by name, but by internal integer value.
	// I'm a bit unclear how any Sheep caller would know this value, and how
	// this value would be persistant across runs of the game...
	std::cout << "GetFlagInt was called!" << std::endl;
	return 0;
}
RegFunc1(GetFlagInt, int, int, IMMEDIATE, REL_FUNC);

shpvoid SetFlag(string flagName)
{
	Services::Get<GameProgress>()->SetFlag(flagName);
	return 0;
}
RegFunc1(SetFlag, void, string, IMMEDIATE, REL_FUNC);

int GetChatCount(string noun)
{
	return Services::Get<GameProgress>()->GetChatCount(noun);
}
RegFunc1(GetChatCount, int, string, IMMEDIATE, REL_FUNC);

int GetChatCountInt(int nounEnum)
{
	//TODO: Should be able to call this from sheep using $n variable.
	return 0;
}
RegFunc1(GetChatCountInt, int, int, IMMEDIATE, REL_FUNC);

shpvoid SetChatCount(string noun, int count) // DEV
{
	Services::Get<GameProgress>()->SetChatCount(noun, count);
	return 0;
}
RegFunc2(SetChatCount, void, string, int, IMMEDIATE, DEV_FUNC);

int GetGameVariableInt(string varName)
{
	return Services::Get<GameProgress>()->GetGameVariable(varName);
}
RegFunc1(GetGameVariableInt, int, string, IMMEDIATE, REL_FUNC);

shpvoid IncGameVariableInt(string varName)
{
	Services::Get<GameProgress>()->IncGameVariable(varName);
	return 0;
}
RegFunc1(IncGameVariableInt, void, string, IMMEDIATE, REL_FUNC);

shpvoid SetGameVariableInt(string varName, int value)
{
	Services::Get<GameProgress>()->SetGameVariable(varName, value);
	return 0;
}
RegFunc2(SetGameVariableInt, void, string, int, IMMEDIATE, REL_FUNC);

int GetNounVerbCount(string noun, string verb)
{
	return Services::Get<GameProgress>()->GetNounVerbCount(noun, verb);
}
RegFunc2(GetNounVerbCount, int, string, string, IMMEDIATE, REL_FUNC);

int GetNounVerbCountInt(int nounEnum, int verbEnum)
{
	//TODO: Should be able to call this from sheep using $n and $v variables.
	return 0;
}
RegFunc2(GetNounVerbCountInt, int, int, int, IMMEDIATE, REL_FUNC);

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

shpvoid SetNounVerbCountBoth(string noun, string verb, int count)
{
	//TODO: This function is undocumented, so I'm not sure how it differs from SetNounVerbCount.
	//TODO: It has same return type and arguments. Maybe we track noun/verb counts separately for Gabe/Grace?
	return SetNounVerbCount(noun, verb, count);
}
RegFunc3(SetNounVerbCountBoth, void, string, string, int, IMMEDIATE, REL_FUNC);

shpvoid TriggerNounVerb(string noun, string verb) // DEV
{
	//TODO
	return 0;
}
RegFunc2(TriggerNounVerb, void, string, string, IMMEDIATE, DEV_FUNC);

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

shpvoid SetScore(int score) // DEV
{
	Services::Get<GameProgress>()->SetScore(score);
	return 0;
}
RegFunc1(SetScore, void, int, IMMEDIATE, DEV_FUNC);

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

// INVENTORY
int DoesEgoHaveInvItem(std::string itemName)
{
	return 0;
}
RegFunc1(DoesEgoHaveInvItem, int, string, IMMEDIATE, REL_FUNC);

int DoesGabeHaveInvItem(std::string itemName)
{
	return 0;
}
RegFunc1(DoesGabeHaveInvItem, int, string, IMMEDIATE, REL_FUNC);

int DoesGraceHaveInvItem(std::string itemName)
{
    return 0;
}
RegFunc1(DoesGraceHaveInvItem, int, string, IMMEDIATE, REL_FUNC);

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
	return 0;
}
RegFunc1(DisableStream, void, string, IMMEDIATE, DEV_FUNC);

shpvoid EnableStream(std::string streamName)
{
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
	return 0;
}
RegFunc2(SetStreamFilename, void, string, string, IMMEDIATE, DEV_FUNC);

shpvoid SetStreamFileTruncate(std::string streamName, int truncate)
{
	return 0;
}
RegFunc2(SetStreamFileTruncate, void, string, int, IMMEDIATE, DEV_FUNC);

// SCENE
shpvoid SetLocation(string location)
{
	GEngine::inst->LoadScene(location);
	return 0;
}
RegFunc1(SetLocation, void, string, WAITABLE, REL_FUNC);
