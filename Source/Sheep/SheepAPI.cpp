#include "SheepAPI.h"

#include <functional> // for std::hash
#include <sstream> // for int->hex

#include "Animator.h"
#include "Camera.h"
#include "CharacterManager.h"
#include "DialogueManager.h"
#include "FaceController.h"
#include "GameCamera.h"
#include "GameProgress.h"
#include "GEngine.h"
#include "GKActor.h"
#include "InventoryManager.h"
#include "LocationManager.h"
#include "Random.h"
#include "Scene.h"
#include "Services.h"
#include "SoundtrackPlayer.h"
#include "StringUtil.h"
#include "Timers.h"
#include "VerbManager.h"
#include "VideoPlayer.h"

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

// Helper for reporting back to Sheep VM that an error occurred.
void ExecError()
{
	Services::GetSheep()->FlagExecutionError();
}

// ACTORS
shpvoid Blink(std::string actorName)
{
	GKActor* actor = GEngine::Instance()->GetScene()->GetActorByNoun(actorName);
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

shpvoid BlinkX(std::string actorName, std::string blinkAnim)
{
	GKActor* actor = GEngine::Instance()->GetScene()->GetActorByNoun(actorName);
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

shpvoid ClearMood(std::string actorName)
{
	// Get actor and make sure it's valid.
	GKActor* actor = GEngine::Instance()->GetScene()->GetActorByNoun(actorName);
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

shpvoid EnableEyeJitter(std::string actorName)
{
	GKActor* actor = GEngine::Instance()->GetScene()->GetActorByNoun(actorName);
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

shpvoid DisableEyeJitter(std::string actorName)
{
	GKActor* actor = GEngine::Instance()->GetScene()->GetActorByNoun(actorName);
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

shpvoid EyeJitter(std::string actorName)
{
	GKActor* actor = GEngine::Instance()->GetScene()->GetActorByNoun(actorName);
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

shpvoid DumpActorPosition(std::string actorName)
{
	GKActor* actor = GEngine::Instance()->GetScene()->GetActorByNoun(actorName);
	if(actor != nullptr)
	{
		actor->DumpPosition();
	}
	else
	{
		ExecError();
	}
    return 0;
}
RegFunc1(DumpActorPosition, void, string, IMMEDIATE, DEV_FUNC);

shpvoid Expression(std::string actorName, std::string expression)
{
	GKActor* actor = GEngine::Instance()->GetScene()->GetActorByNoun(actorName);
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

int GetEgoCurrentLocationCount()
{
	return GetEgoLocationCount(Services::Get<LocationManager>()->GetLocation());
}
RegFunc0(GetEgoCurrentLocationCount, int, IMMEDIATE, REL_FUNC);

int GetEgoLocationCount(std::string locationName)
{
	// Make sure it's a valid location.
	if(!Services::Get<LocationManager>()->IsValidLocation(locationName))
	{
		ExecError();
		return 0;
	}
	
	// Get it!
	const std::string& egoName = GEngine::Instance()->GetScene()->GetEgoName();
	return Services::Get<LocationManager>()->GetLocationCountForCurrentTimeblock(egoName, locationName);
}
RegFunc1(GetEgoLocationCount, int, string, IMMEDIATE, REL_FUNC);

std::string GetEgoName()
{
	return GEngine::Instance()->GetScene()->GetEgoName();
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
    if(!GEngine::Instance()->GetScene()->InitEgoPosition(positionName))
	{
		ExecError();
	}
    return 0;
}
RegFunc1(InitEgoPosition, void, string, IMMEDIATE, REL_FUNC);

int IsActorAtLocation(std::string actorName, std::string locationName)
{
	// Validate actor name.
	if(!Services::Get<CharacterManager>()->IsValidName(actorName))
	{
		ExecError();
		return 0;
	}
	
	// Validate location.
	bool locationValid = Services::Get<LocationManager>()->IsValidLocation(locationName);
	if(!locationValid)
	{
		ExecError();
		return 0;
	}
	
	// See if actor location matches specified location.
	std::string location = Services::Get<LocationManager>()->GetActorLocation(actorName);
	return StringUtil::EqualsIgnoreCase(location, locationName) ? 1 : 0;
}
RegFunc2(IsActorAtLocation, int, string, string, IMMEDIATE, REL_FUNC);

int IsActorNear(std::string actorName, std::string positionName, float distance)
{
	// Make sure distance is valid.
	if(distance < 0.0f)
	{
		Services::GetReports()->Log("Warning", StringUtil::Format("Warning: distance of %f is not valid - must be >= 0.", distance));
		ExecError();
		return 0;
	}
	
	// Get actor and position, or fail.
	GKActor* actor = GEngine::Instance()->GetScene()->GetActorByNoun(actorName);
	if(actor != nullptr)
	{
		const ScenePosition* scenePosition = GEngine::Instance()->GetScene()->GetPosition(positionName);
		if(scenePosition != nullptr)
		{
			// Distance check.
			return (actor->GetPosition() - scenePosition->position).GetLengthSq() < distance * distance;
		}
	}
	
	// Something was null...
	ExecError();
	return 0;
}
RegFunc3(IsActorNear, int, string, string, float, IMMEDIATE, REL_FUNC);

int IsWalkingActorNear(std::string actorName, std::string positionName, float distance)
{
	// Make sure distance is valid.
	if(distance < 0.0f)
	{
		Services::GetReports()->Log("Warning", StringUtil::Format("Warning: distance of %f is not valid - must be >= 0.", distance));
		ExecError();
		return 0;
	}
	
	// Get actor and position, or fail.
	GKActor* actor = GEngine::Instance()->GetScene()->GetActorByNoun(actorName);
	if(actor != nullptr)
	{
		const ScenePosition* scenePosition = GEngine::Instance()->GetScene()->GetPosition(positionName);
		if(scenePosition != nullptr)
		{
			return (actor->GetWalkDestination() - scenePosition->position).GetLengthSq() < distance * distance;
		}
	}
	
	// Something was null...
	ExecError();
	return 0;
}
RegFunc3(IsWalkingActorNear, int, string, string, float, IMMEDIATE, REL_FUNC);
 
int IsActorOffstage(std::string actorName)
{
	// Validate actor name.
	if(!Services::Get<CharacterManager>()->IsValidName(actorName))
	{
		ExecError();
		return 0;
	}
	
	return Services::Get<LocationManager>()->IsActorOffstage(actorName) ? 1 : 0;
}
RegFunc1(IsActorOffstage, int, string, IMMEDIATE, REL_FUNC);

int IsCurrentEgo(string actorName)
{
	const std::string& egoName = GEngine::Instance()->GetScene()->GetEgoName();
	return StringUtil::EqualsIgnoreCase(egoName, actorName) ? 1 : 0;
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
	// Validate actor name.
	if(!Services::Get<CharacterManager>()->IsValidName(actorName))
	{
		ExecError();
		return 0;
	}
	
	// Validate location.
	if(!Services::Get<LocationManager>()->IsValidLocation(locationName))
	{
		ExecError();
		return 0;
	}
	
	Services::Get<LocationManager>()->SetActorLocation(actorName, locationName);
	return 0;
}
RegFunc2(SetActorLocation, void, string, string, IMMEDIATE, REL_FUNC);

shpvoid SetActorOffstage(string actorName)
{
	// Validate actor name.
	if(!Services::Get<CharacterManager>()->IsValidName(actorName))
	{
		ExecError();
		return 0;
	}
	
	Services::Get<LocationManager>()->SetActorOffstage(actorName);
	return 0;
}
RegFunc1(SetActorOffstage, void, string, IMMEDIATE, REL_FUNC);

shpvoid SetActorPosition(std::string actorName, std::string positionName)
{
	// Get needed data.
	GKActor* actor = GEngine::Instance()->GetScene()->GetActorByNoun(actorName);
	const ScenePosition* scenePosition = GEngine::Instance()->GetScene()->GetPosition(positionName);
	
	// If either is null, log an error.
	if(actor == nullptr || scenePosition == nullptr)
	{
		ExecError();
		return 0;
	}
	
	// Docs are unclear about this, but in GK3, this definitely also sets heading.
	actor->SetPosition(scenePosition->position);
	actor->SetHeading(scenePosition->heading);
	return 0;
}
RegFunc2(SetActorPosition, void, string, string, IMMEDIATE, REL_FUNC);

// SetEyeOffsets

// SetEgo

shpvoid SetEgoLocationCount(std::string locationName, int count)
{
	// Make sure it's a valid location.
	if(!Services::Get<LocationManager>()->IsValidLocation(locationName))
	{
		ExecError();
		return 0;
	}
	
	// Set it!
	const std::string& egoName = GEngine::Instance()->GetScene()->GetEgoName();
	Services::Get<LocationManager>()->SetLocationCountForCurrentTimeblock(egoName, locationName, count);
	return 0;
}
RegFunc2(SetEgoLocationCount, void, string, int, IMMEDIATE, DEV_FUNC);

shpvoid SetIdleGAS(std::string actorName, std::string gasName)
{
	// Get actor and make sure it's valid.
	GKActor* actor = GEngine::Instance()->GetScene()->GetActorByNoun(actorName);
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
	GAS* fidget = Services::GetAssets()->LoadGAS(gasName);
	if(fidget == nullptr)
	{
        Services::GetReports()->Log("Error", "Attempted to load an invalid fidget file: " + gasName);
	}
	actor->SetIdleFidget(fidget);
	return 0;
}
RegFunc2(SetIdleGAS, void, string, string, WAITABLE, REL_FUNC); // NOTE: function is waitable, but does nothing.

shpvoid SetListenGAS(std::string actorName, std::string gasName)
{
	// Get actor and make sure it's valid.
	GKActor* actor = GEngine::Instance()->GetScene()->GetActorByNoun(actorName);
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
    GAS* fidget = Services::GetAssets()->LoadGAS(gasName);
    if(fidget == nullptr)
    {
        Services::GetReports()->Log("Error", "Attempted to load an invalid fidget file: " + gasName);
    }
    actor->SetListenFidget(fidget);
    return 0;
}
RegFunc2(SetListenGAS, void, string, string, WAITABLE, REL_FUNC); // NOTE: function is waitable, but does nothing.

shpvoid SetTalkGAS(std::string actorName, std::string gasName)
{
	// Get actor and make sure it's valid.
	GKActor* actor = GEngine::Instance()->GetScene()->GetActorByNoun(actorName);
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
    GAS* fidget = Services::GetAssets()->LoadGAS(gasName);
    if(fidget == nullptr)
    {
        Services::GetReports()->Log("Error", "Attempted to load an invalid fidget file: " + gasName);
    }
    actor->SetTalkFidget(fidget);
	return 0;
}
RegFunc2(SetTalkGAS, void, string, string, WAITABLE, REL_FUNC); // NOTE: function is waitable, but does nothing.

shpvoid SetMood(std::string actorName, std::string moodName)
{
	// Get actor and make sure it's valid.
	GKActor* actor = GEngine::Instance()->GetScene()->GetActorByNoun(actorName);
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

/*
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
	GKActor* actor = GEngine::Instance()->GetScene()->GetActorByNoun(actorName);
	if(actor == nullptr)
	{
        ExecError();
        return 0;
	}

    actor->StartFidget(GKActor::FidgetType::Idle);
	return 0;
}
RegFunc1(StartIdleFidget, void, string, WAITABLE, REL_FUNC);

shpvoid StartListenFidget(std::string actorName)
{
    GKActor* actor = GEngine::Instance()->GetScene()->GetActorByNoun(actorName);
    if(actor == nullptr)
    {
        ExecError();
        return 0;
    }

    actor->StartFidget(GKActor::FidgetType::Listen);
	return 0;
}
RegFunc1(StartListenFidget, void, string, WAITABLE, REL_FUNC);

shpvoid StartTalkFidget(std::string actorName)
{
    GKActor* actor = GEngine::Instance()->GetScene()->GetActorByNoun(actorName);
    if(actor == nullptr)
    {
        ExecError();
        return 0;
    }

    actor->StartFidget(GKActor::FidgetType::Talk);
	return 0;
}
RegFunc1(StartTalkFidget, void, string, WAITABLE, REL_FUNC);

shpvoid StopFidget(std::string actorName)
{
    GKActor* actor = GEngine::Instance()->GetScene()->GetActorByNoun(actorName);
    if(actor == nullptr)
    {
        ExecError();
        return 0;
    }

    SheepThread* currentThread = Services::GetSheep()->GetCurrentThread();
    actor->StopFidget(currentThread->AddWait());
	return 0;
}
RegFunc1(StopFidget, void, string, WAITABLE, REL_FUNC);
 
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
*/
 
shpvoid WalkerBoundaryBlockModel(std::string modelName)
{
	std::cout << "WalkerBoundaryBlockModel" << std::endl;
	return 0;
}
RegFunc1(WalkerBoundaryBlockModel, void, string, IMMEDIATE, REL_FUNC);

/*
shpvoid WalkerBoundaryBlockRegion(int regionIndex, int regionBoundaryIndex)
{
	std::cout << "WalkerBoundaryBlockRegion" << std::endl;
	return 0;
}
RegFunc2(WalkerBoundaryBlockRegion, void, int, int, IMMEDIATE, REL_FUNC);
*/
 
shpvoid WalkerBoundaryUnblockModel(std::string modelName)
{
	std::cout << "WalkerBoundaryUnblockModel" << std::endl;
	return 0;
}
RegFunc1(WalkerBoundaryUnblockModel, void, string, IMMEDIATE, REL_FUNC);

/*
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
*/

shpvoid WalkTo(std::string actorName, std::string positionName)
{
    // If not in a scene, we'll just ignore walk to request.
    Scene* scene = GEngine::Instance()->GetScene();
    if(scene == nullptr)
    {
        return 0;
    }

    // Get actor.
    GKActor* actor = scene->GetActorByNoun(actorName);
    if(actor == nullptr)
    {
        ExecError();
        return 0;
    }

    // Get position.
    const ScenePosition* scenePosition = scene->GetPosition(positionName);
    if(scenePosition == nullptr)
    {
        ExecError();
        return 0;
    }
    
    // Ok, we can actually do the walk to it seems!
    SheepThread* currentThread = Services::GetSheep()->GetCurrentThread();
    actor->WalkTo(scenePosition->position, scenePosition->heading, currentThread->AddWait());
	return 0;
}
RegFunc2(WalkTo, void, string, string, WAITABLE, REL_FUNC);

shpvoid WalkToAnimation(std::string actorName, std::string animationName)
{
    // If not in a scene, we'll just ignore walk to request.
    Scene* scene = GEngine::Instance()->GetScene();
    if(scene == nullptr)
    {
        return 0;
    }

    // Get actor.
    GKActor* actor = scene->GetActorByNoun(actorName);
    if(actor == nullptr)
    {
        ExecError();
        return 0;
    }

    // Get the animation.
    Animation* anim = Services::GetAssets()->LoadAnimation(animationName);
    if(anim == nullptr)
    {
        ExecError();
        return 0;
    }

    // Ok, we can actually do the walk to it seems!
    SheepThread* currentThread = Services::GetSheep()->GetCurrentThread();
    actor->WalkToAnimationStart(anim, currentThread->AddWait());
	return 0;
}
RegFunc2(WalkToAnimation, void, string, string, WAITABLE, REL_FUNC);

/*
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
	// Make sure it's a valid location.
	if(!Services::Get<LocationManager>()->IsValidLocation(locationName))
	{
		Services::GetReports()->Log("Error", "Error: '" + locationName + "' is not a valid location name. Call DumpLocations() to see valid locations.");
		
		std::string sheepContextName = GetCurrentSheepName() + ":" + GetCurrentSheepFunction();
		Services::GetReports()->Log("Error", "An error occurred while executing " + sheepContextName);
		return 0;
	}
	
	// Returns if Ego was EVER in a location during ANY timeblock!
	const std::string& egoName = GEngine::Instance()->GetScene()->GetEgoName();
	int locationCount = Services::Get<LocationManager>()->GetLocationCountAcrossAllTimeblocks(egoName, locationName);
	return locationCount > 0 ? 1 : 0;
}
RegFunc1(WasEgoEverInLocation, int, string, IMMEDIATE, REL_FUNC);

// ANIMATION AND DIALOGUE

//AddCaptionDefault

//AddCaptionEgo

//AddCaptionVoiceOver

//ClearCaptionText

//AnimEvent

shpvoid StartDialogue(std::string licensePlate, int numLines)
{
	SheepThread* currentThread = Services::GetSheep()->GetCurrentThread();
	Services::Get<DialogueManager>()->StartDialogue(licensePlate, numLines, true, currentThread->AddWait());
    return 0;
}
RegFunc2(StartDialogue, void, string, int, WAITABLE, REL_FUNC);

shpvoid StartDialogueNoFidgets(std::string licensePlate, int numLines)
{
	SheepThread* currentThread = Services::GetSheep()->GetCurrentThread();
	Services::Get<DialogueManager>()->StartDialogue(licensePlate, numLines, false, currentThread->AddWait());
    return 0;
}
RegFunc2(StartDialogueNoFidgets, void, string, int, WAITABLE, REL_FUNC);

shpvoid ContinueDialogue(int numLines)
{
	SheepThread* currentThread = Services::GetSheep()->GetCurrentThread();
	Services::Get<DialogueManager>()->ContinueDialogue(numLines, true, currentThread->AddWait());
	return 0;
}
RegFunc1(ContinueDialogue, void, int, WAITABLE, REL_FUNC);

shpvoid ContinueDialogueNoFidgets(int numLines)
{
	SheepThread* currentThread = Services::GetSheep()->GetCurrentThread();
	Services::Get<DialogueManager>()->ContinueDialogue(numLines, false, currentThread->AddWait());
	return 0;
}
RegFunc1(ContinueDialogueNoFidgets, void, int, WAITABLE, REL_FUNC);
 
//EnableInterpolation

//DisableInterpolation

//DumpAnimator

shpvoid SetConversation(std::string conversationName)
{
    SheepThread* currentThread = Services::GetSheep()->GetCurrentThread();
	Services::Get<DialogueManager>()->SetConversation(conversationName, currentThread->AddWait());
	return 0;
}
RegFunc1(SetConversation, void, string, WAITABLE, REL_FUNC);

shpvoid EndConversation()
{
    SheepThread* currentThread = Services::GetSheep()->GetCurrentThread();
	Services::Get<DialogueManager>()->EndConversation(currentThread->AddWait());
	return 0;
}
RegFunc0(EndConversation, void, WAITABLE, REL_FUNC);

shpvoid SetDefaultDialogueCamera(std::string cameraName)
{
	return 0;
}
RegFunc1(SetDefaultDialogueCamera, void, string, IMMEDIATE, REL_FUNC);
 
shpvoid StartAnimation(std::string animationName)
{
	Animation* animation = Services::GetAssets()->LoadAnimation(animationName);
    if(animation == nullptr)
    {
        Services::GetReports()->Log("Error", "gk3 animation '" + animationName + ".anm' not found.");
        return 0;
    }
    
    SheepThread* currentThread = Services::GetSheep()->GetCurrentThread();
    GEngine::Instance()->GetScene()->GetAnimator()->Start(animation, currentThread->AddWait());
	return 0;
}
RegFunc1(StartAnimation, void, string, WAITABLE, REL_FUNC);

shpvoid StartMoveAnimation(std::string animationName)
{
	Animation* animation = Services::GetAssets()->LoadAnimation(animationName);
	if(animation != nullptr)
	{
		SheepThread* currentThread = Services::GetSheep()->GetCurrentThread();
		GEngine::Instance()->GetScene()->GetAnimator()->Start(animation, currentThread->AddWait());
	}
	return 0;
}
RegFunc1(StartMoveAnimation, void, string, WAITABLE, REL_FUNC);

shpvoid StartMom(std::string momAnimationName)
{
    // Mom animation assets have a language prefix (e.g. "E" for English).
    // So, let's add that here.
    momAnimationName = "E" + momAnimationName;

    Animation* animation = Services::GetAssets()->LoadMomAnimation(momAnimationName);
    if(animation != nullptr)
    {
        //TODO: Any need to send flag that this is a MOM animation file? The formats/uses seem identical.
        SheepThread* currentThread = Services::GetSheep()->GetCurrentThread();
        GEngine::Instance()->GetScene()->GetAnimator()->Start(animation, currentThread->AddWait());
    }
	return 0;
}
RegFunc1(StartMom, void, string, WAITABLE, REL_FUNC);
 
shpvoid LoopAnimation(std::string animationName)
{
	Animation* animation = Services::GetAssets()->LoadAnimation(animationName);
	if(animation != nullptr)
	{
        AnimParams params;
        params.animation = animation;
        params.loop = true;
		GEngine::Instance()->GetScene()->GetAnimator()->Start(params);
	}
	return 0;
}
RegFunc1(LoopAnimation, void, string, IMMEDIATE, REL_FUNC);

shpvoid StopAnimation(std::string animationName)
{
	Animation* animation = Services::GetAssets()->LoadAnimation(animationName);
	if(animation != nullptr)
	{
		GEngine::Instance()->GetScene()->GetAnimator()->Stop(animation);
	}
	return 0;
}
RegFunc1(StopAnimation, void, string, IMMEDIATE, REL_FUNC);

shpvoid StopAllAnimations()
{
    GEngine::Instance()->GetScene()->GetAnimator()->StopAll();
    return 0;
}
RegFunc0(StopAllAnimations, void, IMMEDIATE, DEV_FUNC);

/*
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
	if(numLines > 1)
	{
		std::cout << "StartVoiceOver lines were greater than 1." << std::endl;
	}
    std::string yakName = "E" + dialogueName;
	Animation* yak = Services::GetAssets()->LoadYak(yakName);
	
	SheepThread* currentThread = Services::GetSheep()->GetCurrentThread();

    AnimParams params;
    params.animation = yak;
    params.finishCallback = currentThread->AddWait();
    params.isYak = true;
    GEngine::Instance()->GetScene()->GetAnimator()->Start(params);
    return 0;
}
RegFunc2(StartVoiceOver, void, string, int, WAITABLE, REL_FUNC);

shpvoid StartYak(string yakAnimationName)
{
	Animation* yak = Services::GetAssets()->LoadYak(yakAnimationName);
	SheepThread* currentThread = Services::GetSheep()->GetCurrentThread();

    AnimParams params;
    params.animation = yak;
    params.finishCallback = currentThread->AddWait();
    params.isYak = true;
    GEngine::Instance()->GetScene()->GetAnimator()->Start(params);
    return 0;
}
RegFunc1(StartYak, void, string, WAITABLE, DEV_FUNC);

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
*/

shpvoid DumpLayerStack()
{
    Services::Get<LayerManager>()->DumpLayerStack();
    return 0;
}
RegFunc0(DumpLayerStack, void, IMMEDIATE, DEV_FUNC);

/*
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
	GEngine::Instance()->Quit();
	return 0;
}
RegFunc0(ForceQuitGame, void, IMMEDIATE, DEV_FUNC);

shpvoid QuitApp()
{
	//TODO: Should quit nicely by asking "are you sure" and making sure you saved.
	GEngine::Instance()->Quit();
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
	GEngine::Instance()->GetScene()->SetCameraPosition(cameraName);
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
	GEngine::Instance()->GetScene()->SetCameraPosition(cameraName);
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
*/
 
shpvoid EnableCameraBoundaries()
{
	GEngine::Instance()->GetScene()->GetCamera()->SetBoundsEnabled(true);
	return 0;
}
RegFunc0(EnableCameraBoundaries, void, IMMEDIATE, DEV_FUNC);
 
shpvoid DisableCameraBoundaries()
{
	// Note that this only disables camera boundaries until the next scene load.
	// This reflects the behavior in the OG game.
	GEngine::Instance()->GetScene()->GetCamera()->SetBoundsEnabled(false);
	return 0;
}
RegFunc0(DisableCameraBoundaries, void, IMMEDIATE, DEV_FUNC);

//DisableCameraBoundaries

//DumpCamera
//DumpCameraAngles

//GetCameraAngleCount
//GetIndexedCameraAngle

/*
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
*/
 
shpvoid InspectObject()
{
	//TODO: What is "current object"? This just seems to output an error when run in OG game.
	return 0;
}
RegFunc0(InspectObject, void, WAITABLE, REL_FUNC);

/*
shpvoid Uninspect()
{
	// Glide back to camera position/heading before inspecting object.
	return 0;
}
RegFunc0(Uninspect, void, WAITABLE, REL_FUNC);
*/
 
float GetCameraFOV()
{
	// Dig down to grab the value.
	GameCamera* gameCamera = GEngine::Instance()->GetScene()->GetCamera();
	if(gameCamera != nullptr)
	{
		Camera* camera = gameCamera->GetCamera();
		if(camera != nullptr)
		{
			return camera->GetCameraFovDegrees();
		}
	}
	return 0.0f;
}
RegFunc0(GetCameraFOV, float, IMMEDIATE, REL_FUNC);

shpvoid SetCameraFOV(float fov)
{
	// Clamp argument in valid range.
	if(fov < 1.0f || fov > 180.0f)
	{
		Services::GetReports()->Log("Warning", "Warning: camera FOV must be between 1 and 180. Clamping to fit...");
		fov = Math::Clamp(fov, 1.0f, 180.0f);
	}
	
	// Dig down to actually set it.
	GameCamera* gameCamera = GEngine::Instance()->GetScene()->GetCamera();
	if(gameCamera != nullptr)
	{
		Camera* camera = gameCamera->GetCamera();
		if(camera != nullptr)
		{
			camera->SetCameraFovDegrees(fov);
		}
	}
	return 0;
}
RegFunc1(SetCameraFOV, void, float, IMMEDIATE, REL_FUNC);

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
	if(currentThread != nullptr && currentThread->mContext != nullptr)
	{
		SheepScript* sheep = currentThread->mContext->mSheepScript;
		if(sheep != nullptr)
		{
			Services::GetSheep()->Execute(sheep, functionName, currentThread != nullptr ? currentThread->AddWait() : nullptr);
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

shpvoid Extract(std::string fileSpec, std::string outputPath)
{
	//TODO: fileSpec should support regex if surrounded by { }.
	//TODO: fileSpec prefixed with @ indicates a "listing file" - what's a listing file? Perhaps a file that lists assets to extract?
	
	// An empty output path defaults to EXE directory.
	// If a path is provided, absolute or relative paths will work.
	if(outputPath.empty())
	{
		Services::GetAssets()->WriteBarnAssetToFile(fileSpec);
	}
	else
	{
		Services::GetAssets()->WriteBarnAssetToFile(fileSpec, outputPath);
	}
	return 0;
}
RegFunc2(Extract, void, string, string, IMMEDIATE, REL_FUNC);

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
	return currentThread != nullptr ? currentThread->mFunctionName : "";
}
RegFunc0(GetCurrentSheepFunction, string, IMMEDIATE, REL_FUNC);

std::string GetCurrentSheepName()
{
	SheepThread* currentThread = Services::GetSheep()->GetCurrentThread();
	if(currentThread != nullptr && currentThread->mContext != nullptr)
	{
		// Specifically, return name with no extension.
		return currentThread->mContext->mSheepScript->GetNameNoExtension();
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

shpvoid SetTimerMs(int milliseconds)
{
    // Should throw error if not waited upon!
    SheepThread* currentThread = Services::GetSheep()->GetCurrentThread();
    if(!currentThread->mInWaitBlock)
    {
        Services::GetReports()->Log("Warning", "No point setting a timer if you don't wait for it to finish. " +
                                                std::to_string(milliseconds) + " millisecond timer request ignored.");
        ExecError();
    }
    else
    {
        Timers::AddTimerMilliseconds(static_cast<unsigned int>(milliseconds), currentThread->AddWait());
    }
	return 0;
}
RegFunc1(SetTimerMs, void, int, WAITABLE, REL_FUNC);

shpvoid SetTimerSeconds(float seconds)
{
    // Should throw error if not waited upon!
    SheepThread* currentThread = Services::GetSheep()->GetCurrentThread();
    if(!currentThread->mInWaitBlock)
    {
        Services::GetReports()->Log("Warning", "No point setting a timer if you don't wait for it to finish. " +
                                    std::to_string(seconds) + " second timer request ignored.");
        ExecError();
    }
    else
    {
        Timers::AddTimerSeconds(seconds, currentThread->AddWait());
    }
    return 0;
}
RegFunc1(SetTimerSeconds, void, float, WAITABLE, REL_FUNC);
 
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
	//TODO: Throw an error if the given noun corresponds to a "Topic".
	Services::Get<GameProgress>()->IncNounVerbCount(noun, verb);
	return 0;
}
RegFunc2(IncNounVerbCount, void, string, string, IMMEDIATE, REL_FUNC);

shpvoid IncNounVerbCountBoth(string noun, string verb)
{
	//TODO: HelpCommand says this sets the noun/verb count for both Gabe and Grace.
	//TODO: Does that imply SetNounVerbCount tracks per-Ego?
	Services::Get<GameProgress>()->IncNounVerbCount(noun, verb);
	return 0;
}
RegFunc2(IncNounVerbCountBoth, void, string, string, IMMEDIATE, REL_FUNC);

shpvoid SetNounVerbCount(string noun, string verb, int count)
{
	//TODO: Throw an error if the given noun corresponds to a "Topic".
	Services::Get<GameProgress>()->SetNounVerbCount(noun, verb, count);
	return 0;
}
RegFunc3(SetNounVerbCount, void, string, string, int, IMMEDIATE, REL_FUNC);

shpvoid SetNounVerbCountBoth(string noun, string verb, int count)
{
	//TODO: HelpCommand says this sets the noun/verb count for both Gabe and Grace.
	//TODO: Does that imply SetNounVerbCount tracks per-Ego?
	return SetNounVerbCount(noun, verb, count);
}
RegFunc3(SetNounVerbCountBoth, void, string, string, int, IMMEDIATE, REL_FUNC);

shpvoid TriggerNounVerb(std::string noun, std::string verb)
{
	//TODO: Validate noun or throw error.
	//TODO: Validate verb or throw error.
	bool success = Services::Get<ActionManager>()->ExecuteAction(noun, verb);
	if(!success)
	{
		Services::GetReports()->Log("Error", "Error: unable to trigger noun-verb combination " + noun + ":" + verb);
	}
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

shpvoid SetScore(int score)
{
	Services::Get<GameProgress>()->SetScore(score);
	return 0;
}
RegFunc1(SetScore, void, int, IMMEDIATE, DEV_FUNC);

shpvoid ChangeScore(std::string scoreValue)
{
	std::cout << "ChangeScore " << scoreValue << std::endl;
	return 0;
}
RegFunc1(ChangeScore, void, string, IMMEDIATE, REL_FUNC);

int GetTopicCount(std::string noun, std::string verb)
{
	//TODO: Validate noun. Must be a valid noun. Seems to include any scene nouns, inventory nouns, actor nouns.
	if(!Services::Get<VerbManager>()->IsTopic(verb))
	{
		Services::GetReports()->Log("Error", "Error: '" + verb + " is not a valid verb name.");
		return 0;
	}
	return Services::Get<GameProgress>()->GetTopicCount(noun, verb);
}
RegFunc2(GetTopicCount, int, string, string, IMMEDIATE, REL_FUNC);

int GetTopicCountInt(int nounEnum, int verbEnum)
{
	std::string noun = Services::Get<ActionManager>()->GetNoun(nounEnum);
	std::string verb = Services::Get<ActionManager>()->GetVerb(verbEnum);
	return GetTopicCount(noun, verb);
}
RegFunc2(GetTopicCountInt, int, int, int, IMMEDIATE, REL_FUNC);
 
int HasTopicsLeft(std::string noun)
{
	//TODO: Validate noun.
	bool hasTopics = Services::Get<ActionManager>()->HasTopicsLeft(noun);
	return hasTopics ? 1 : 0;
}
RegFunc1(HasTopicsLeft, int, string, IMMEDIATE, REL_FUNC);

shpvoid SetTopicCount(std::string noun, std::string verb, int count)
{
	//TODO: Validate noun or report error.
	//TODO: Validate verb or report error.
	Services::Get<GameProgress>()->SetTopicCount(noun, verb, count);
	return 0;
}
RegFunc3(SetTopicCount, void, string, string, int, IMMEDIATE, DEV_FUNC);
 
int IsCurrentLocation(std::string location)
{
	std::string currentLocation = Services::Get<LocationManager>()->GetLocation();
	return StringUtil::EqualsIgnoreCase(currentLocation, location) ? 1 : 0;
}
RegFunc1(IsCurrentLocation, int, string, IMMEDIATE, REL_FUNC);

int IsCurrentTime(std::string timeblock)
{
	std::string currentTimeblock = Services::Get<GameProgress>()->GetTimeblock().ToString();
	return StringUtil::EqualsIgnoreCase(currentTimeblock, timeblock) ? 1 : 0;
}
RegFunc1(IsCurrentTime, int, string, IMMEDIATE, REL_FUNC);

int WasLastLocation(std::string location)
{
	std::string lastLocation = Services::Get<LocationManager>()->GetLastLocation();
	return StringUtil::EqualsIgnoreCase(lastLocation, location) ? 1 : 0;
}
RegFunc1(WasLastLocation, int, string, IMMEDIATE, REL_FUNC);

int WasLastTime(std::string timeblock)
{
	std::string lastTimeblock = Services::Get<GameProgress>()->GetLastTimeblock().ToString();
	return StringUtil::EqualsIgnoreCase(lastTimeblock, timeblock) ? 1 : 0;
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
RegFunc2(GetRandomInt, int, int, int, IMMEDIATE, REL_FUNC);

shpvoid PlayFullScreenMovie(std::string movieName)
{
    SheepThread* currentThread = Services::GetSheep()->GetCurrentThread();
    Services::Get<VideoPlayer>()->Play(movieName, true, true, currentThread->AddWait());
	return 0;
}
RegFunc1(PlayFullScreenMovie, void, string, WAITABLE, REL_FUNC);

shpvoid PlayFullScreenMovieX(std::string movieName, int autoclose)
{
    SheepThread* currentThread = Services::GetSheep()->GetCurrentThread();
    Services::Get<VideoPlayer>()->Play(movieName, true, autoclose != 0 ? true : false, currentThread->AddWait());
    return 0;
}
RegFunc2(PlayFullScreenMovieX, void, string, int, WAITABLE, REL_FUNC);

shpvoid PlayMovie(std::string movieName)
{
    SheepThread* currentThread = Services::GetSheep()->GetCurrentThread();
    Services::Get<VideoPlayer>()->Play(movieName, false, true, currentThread->AddWait());
	return 0;
}
RegFunc1(PlayMovie, void, string, WAITABLE, REL_FUNC);

/*
shpvoid SetPamphletPage(int page)
{
	std::cout << "SetPamphletPage" << std::endl;
	return 0;
}
RegFunc1(SetPamphletPage, void, int, WAITABLE, REL_FUNC);
*/
 
// INSETS
//DumpInsetNames
/*
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
*/
 
// INVENTORY
shpvoid CombineInvItems(std::string firstItemName, std::string secondItemName,
						std::string combinedItemName)
{
	// All three items must be valid.
	// Output error messages for ALL invalid items before early out.
	bool firstItemValid = Services::Get<InventoryManager>()->IsValidInventoryItem(firstItemName);
	bool secondItemValid = Services::Get<InventoryManager>()->IsValidInventoryItem(secondItemName);
	bool combinedItemValid = Services::Get<InventoryManager>()->IsValidInventoryItem(combinedItemName);
	if(!firstItemValid)
	{
		Services::GetReports()->Log("Error", "'" + firstItemName + "' is not a valid inventory item name.");
	}
	if(!secondItemValid)
	{
		Services::GetReports()->Log("Error", "'" + secondItemName + "' is not a valid inventory item name.");
	}
	if(!combinedItemValid)
	{
		Services::GetReports()->Log("Error", "'" + combinedItemName + "' is not a valid inventory item name.");
	}
	if(!firstItemValid || !secondItemValid || !combinedItemValid)
	{
		return 0;
	}
	
	// This function doesn't actually check whether you HAVE any of the items involved in the combining or output.
	// It simply removes the first two (whether they exist or not) and adds the combined (whether you already have it or not).
	const std::string& egoName = GEngine::Instance()->GetScene()->GetEgoName();
	Services::Get<InventoryManager>()->RemoveInventoryItem(egoName, firstItemName);
	Services::Get<InventoryManager>()->RemoveInventoryItem(egoName, secondItemName);
	Services::Get<InventoryManager>()->AddInventoryItem(egoName, combinedItemName);
	return 0;
}
RegFunc3(CombineInvItems, void, string, string, string, IMMEDIATE, REL_FUNC);

int DoesEgoHaveInvItem(std::string itemName)
{
	// This function does work with invalid inventory item names.
	const std::string& egoName = GEngine::Instance()->GetScene()->GetEgoName();
	bool hasItem = Services::Get<InventoryManager>()->HasInventoryItem(egoName, itemName);
	return hasItem ? 1 : 0;
}
RegFunc1(DoesEgoHaveInvItem, int, string, IMMEDIATE, REL_FUNC);

int DoesGabeHaveInvItem(std::string itemName)
{
	// This function does work with invalid inventory item names.
	bool hasItem = Services::Get<InventoryManager>()->HasInventoryItem("Gabriel", itemName);
	return hasItem ? 1 : 0;
}
RegFunc1(DoesGabeHaveInvItem, int, string, IMMEDIATE, REL_FUNC);

int DoesGraceHaveInvItem(std::string itemName)
{
	// This function does work with invalid inventory item names.
	bool hasItem = Services::Get<InventoryManager>()->HasInventoryItem("Grace", itemName);
	return hasItem ? 1 : 0;
}
RegFunc1(DoesGraceHaveInvItem, int, string, IMMEDIATE, REL_FUNC);

shpvoid EgoTakeInvItem(std::string itemName)
{
	// It must be a valid inventory item.
	if(!Services::Get<InventoryManager>()->IsValidInventoryItem(itemName))
	{
		Services::GetReports()->Log("Error", "Error: '" + itemName + "' is not a valid inventory item name.");
		return 0;
	}
	
	// Add to inventory of Ego.
	const std::string& egoName = GEngine::Instance()->GetScene()->GetEgoName();
	Services::Get<InventoryManager>()->AddInventoryItem(egoName, itemName);
	return 0;
}
RegFunc1(EgoTakeInvItem, void, string, IMMEDIATE, REL_FUNC);

shpvoid DumpEgoActiveInvItem()
{
	const std::string& egoName = GEngine::Instance()->GetScene()->GetEgoName();
	std::string activeItem = Services::Get<InventoryManager>()->GetActiveInventoryItem(egoName);
	if(activeItem.empty())
	{
		Services::GetReports()->Log("Dump", "Ego active inventory item is 'NONE'.");
	}
	else
	{
		Services::GetReports()->Log("Dump", "Ego active inventory item is '" + activeItem + "'.");
	}
	return 0;
}
RegFunc0(DumpEgoActiveInvItem, void, IMMEDIATE, DEV_FUNC);

shpvoid SetEgoActiveInvItem(std::string itemName)
{
	// It must be a valid inventory item.
	if(!Services::Get<InventoryManager>()->IsValidInventoryItem(itemName))
	{
		Services::GetReports()->Log("Error", "Error: '" + itemName + "' is not a valid inventory item name.");
		return 0;
	}
	
	// If the item we are setting active is not in our inventory, output a warning (but let it go anyway).
	const std::string& egoName = GEngine::Instance()->GetScene()->GetEgoName();
	if(!Services::Get<InventoryManager>()->HasInventoryItem(egoName, itemName))
	{
		Services::GetReports()->Log("Warning", egoName + " does not have " + itemName + ".");
	}
	
	// Set the inventory item!
	Services::Get<InventoryManager>()->SetActiveInventoryItem(egoName, itemName);
	return 0;
}
RegFunc1(SetEgoActiveInvItem, void, string, IMMEDIATE, REL_FUNC);

shpvoid ShowInventory()
{
	Services::Get<InventoryManager>()->ShowInventory();
	return 0;
}
RegFunc0(ShowInventory, void, IMMEDIATE, REL_FUNC);

shpvoid HideInventory()
{
	Services::Get<InventoryManager>()->HideInventory();
	return 0;
}
RegFunc0(HideInventory, void, IMMEDIATE, REL_FUNC);

shpvoid InventoryInspect(std::string itemName)
{
	Services::Get<InventoryManager>()->InventoryInspect(itemName);
	return 0;
}
RegFunc1(InventoryInspect, void, string, IMMEDIATE, REL_FUNC);

shpvoid InventoryUninspect()
{
	Services::Get<InventoryManager>()->InventoryUninspect();
	return 0;
}
RegFunc0(InventoryUninspect, void, IMMEDIATE, REL_FUNC);

shpvoid SetInvItemStatus(std::string itemName, std::string status)
{
	std::cout << "SetInvItemStatus: " << itemName << ", " << status << std::endl;
	
	// The item name must be valid.
	bool argumentError = false;
	if(!Services::Get<InventoryManager>()->IsValidInventoryItem(itemName))
	{
		Services::GetReports()->Log("Error", "Error: '" + itemName + "' is not a valid inventory item name.");
		argumentError = true;
	}
	
	// Make sure we're using a valid status.
	// NOTE: The reason I check inputs before doing the action is to emulate how error output works in the original game.
	bool validStatus = false;
	if(StringUtil::EqualsIgnoreCase(status, "NotPlaced")
	   || StringUtil::EqualsIgnoreCase(status, "Used")
	   || StringUtil::EqualsIgnoreCase(status, "Placed")
	   || StringUtil::EqualsIgnoreCase(status, "GraceHas")
	   || StringUtil::EqualsIgnoreCase(status, "GabeHas")
	   || StringUtil::EqualsIgnoreCase(status, "BothHave")
	   || StringUtil::EqualsIgnoreCase(status, "Used"))
	{
		validStatus = true;
	}
	if(!validStatus)
	{
		Services::GetReports()->Log("Error", "Error: '" + status + "' is not a valid inventory status.");
		argumentError = true;
	}
	
	// Early out if any argument is invalid.
	if(argumentError)
	{
		return 0;
	}
	
	// Status can be any of: NotPlaced, Placed, GraceHas, GabeHas, BothHave, Used.
	// It's unclear what the relevance of these different statuses are - there's no way to 'get' the status of an inventory item!
	// For now, I'll just boil these statuses down to the important states: have or don't have.
	if(StringUtil::EqualsIgnoreCase(status, "NotPlaced")
	   || StringUtil::EqualsIgnoreCase(status, "Placed")
	   || StringUtil::EqualsIgnoreCase(status, "Used"))
	{
		// NotPlaced = not in the game = make sure neither ego has it.
		// Placed = placed in the game, but not in anyone's inventory = make sure neither ego has it.
		// Used = item is used and no longer available = make sure neither ego has it.
		Services::Get<InventoryManager>()->RemoveInventoryItem("Gabriel", itemName);
		Services::Get<InventoryManager>()->RemoveInventoryItem("Grace", itemName);
	}
	else if(StringUtil::EqualsIgnoreCase(status, "GraceHas"))
	{
		// Grace has, but also implies that Gabriel DOES NOT have!
		Services::Get<InventoryManager>()->AddInventoryItem("Grace", itemName);
		Services::Get<InventoryManager>()->RemoveInventoryItem("Gabriel", itemName);
	}
	else if(StringUtil::EqualsIgnoreCase(status, "GabeHas"))
	{
		// Gabe has, but also implies that Grace DOES NOT have!
		Services::Get<InventoryManager>()->AddInventoryItem("Gabriel", itemName);
		Services::Get<InventoryManager>()->RemoveInventoryItem("Grace", itemName);
	}
	else if(StringUtil::EqualsIgnoreCase(status, "BothHave"))
	{
		Services::Get<InventoryManager>()->AddInventoryItem("Gabriel", itemName);
		Services::Get<InventoryManager>()->AddInventoryItem("Grace", itemName);
	}
	return 0;
}
RegFunc2(SetInvItemStatus, void, string, string, IMMEDIATE, REL_FUNC);
 
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
	GKObject* object = GEngine::Instance()->GetScene()->GetSceneObjectByModelName(modelName);
	return object != nullptr ? 1 : 0;
}
RegFunc1(DoesModelExist, int, string, IMMEDIATE, REL_FUNC);

int DoesSceneModelExist(std::string modelName)
{
	return GEngine::Instance()->GetScene()->DoesSceneModelExist(modelName) ? 1 : 0;
}
RegFunc1(DoesSceneModelExist, int, string, IMMEDIATE, REL_FUNC);

//DumpModel
//DumpModelNames
//DumpSceneModelNames

shpvoid ShowModel(std::string modelName)
{
    GKObject* object = GEngine::Instance()->GetScene()->GetSceneObjectByModelName(modelName);
	if(object != nullptr)
	{
		object->SetActive(true);
	}
	return 0;
}
RegFunc1(ShowModel, void, string, IMMEDIATE, REL_FUNC);

shpvoid HideModel(std::string modelName)
{
    GKObject* object = GEngine::Instance()->GetScene()->GetSceneObjectByModelName(modelName);
	if(object != nullptr)
	{
		object->SetActive(false);
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
	GEngine::Instance()->GetScene()->SetSceneModelVisibility(modelName, true);
	return 0;
}
RegFunc1(ShowSceneModel, void, string, IMMEDIATE, REL_FUNC);

shpvoid HideSceneModel(std::string modelName)
{
	GEngine::Instance()->GetScene()->SetSceneModelVisibility(modelName, false);
	return 0;
}
RegFunc1(HideSceneModel, void, string, IMMEDIATE, REL_FUNC);

int IsModelVisible(std::string modelName)
{
    GKObject* object = GEngine::Instance()->GetScene()->GetSceneObjectByModelName(modelName);
	if(object != nullptr)
	{
		return object->IsActive() ? 1 : 0;
	}
	return 0;
}
RegFunc1(IsModelVisible, int, string, IMMEDIATE, REL_FUNC);

int IsSceneModelVisible(std::string modelName)
{
	return GEngine::Instance()->GetScene()->IsSceneModelVisible(modelName) ? 1 : 0;
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
	GEngine::Instance()->LoadScene(location);
	return 0;
}
RegFunc1(SetLocation, void, string, WAITABLE, REL_FUNC);

shpvoid SetLocationTime(std::string location, std::string timeblock)
{
	Services::Get<GameProgress>()->SetTimeblock(Timeblock(timeblock));
	GEngine::Instance()->LoadScene(location);
	return 0;
}
RegFunc2(SetLocationTime, void, string, string, WAITABLE, REL_FUNC);

shpvoid SetTime(std::string timeblock)
{
	// Change time, but load in to the same scene we are currently in.
	Services::Get<GameProgress>()->SetTimeblock(Timeblock(timeblock));
	GEngine::Instance()->LoadScene(Services::Get<LocationManager>()->GetLocation());
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
		Services::GetAudio()->PlaySFX(audio);
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
		GEngine::Instance()->GetScene()->GetSoundtrackPlayer()->Play(soundtrack);
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
shpvoid PrintFloat(float value)
{
	Services::GetReports()->Log("SheepScript", std::to_string(value));
	return 0;
}
RegFunc1(PrintFloat, void, float, IMMEDIATE, DEV_FUNC);

shpvoid PrintFloatX(std::string category, float value)
{
	Services::GetReports()->Log(category, std::to_string(value));
	return 0;
}
RegFunc2(PrintFloatX, void, string, float, IMMEDIATE, DEV_FUNC);

shpvoid PrintInt(int value)
{
	Services::GetReports()->Log("SheepScript", std::to_string(value));
	return 0;
}
RegFunc1(PrintInt, void, int, IMMEDIATE, DEV_FUNC);

shpvoid PrintIntX(std::string category, int value)
{
	Services::GetReports()->Log(category, std::to_string(value));
	return 0;
}
RegFunc2(PrintIntX, void, string, int, IMMEDIATE, DEV_FUNC);

shpvoid PrintIntHex(int value)
{
	std::stringstream ss;
	ss << std::hex << std::setw(2) << std::setfill('0') << std::uppercase << value;
	Services::GetReports()->Log("SheepScript", ss.str());
	return 0;
}
RegFunc1(PrintIntHex, void, int, IMMEDIATE, DEV_FUNC);

shpvoid PrintIntHexX(std::string category, int value)
{
	std::stringstream ss;
	ss << std::hex << std::setw(2) << std::setfill('0') << std::uppercase << value;
	Services::GetReports()->Log(category, ss.str());
	return 0;
}
RegFunc2(PrintIntHexX, void, string, int, IMMEDIATE, DEV_FUNC);

shpvoid PrintString(std::string string)
{
	Services::GetReports()->Log("SheepScript", string);
	return 0;
}
RegFunc1(PrintString, void, string, IMMEDIATE, DEV_FUNC);

shpvoid PrintStringX(std::string category, std::string string)
{
	Services::GetReports()->Log(category, string);
	return 0;
}
RegFunc2(PrintStringX, void, string, string, IMMEDIATE, DEV_FUNC);

// UNDOCUMENTED
int IsTopLayerInventory()
{
    // This returns true if top layer is inventory OR inventory inspect screens.
    bool anyShowing = Services::Get<InventoryManager>()->IsInventoryShowing() ||
                      Services::Get<InventoryManager>()->IsInventoryInspectShowing();
    return anyShowing ? 1 : 0;
}
RegFunc0(IsTopLayerInventory, int, IMMEDIATE, REL_FUNC);
