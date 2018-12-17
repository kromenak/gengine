//
// SheepAPI.cpp
//
// Clark Kromenaker
//
#include "SheepAPI.h"

#include "CharacterManager.h"
#include "GEngine.h"
#include "GKActor.h"
#include "Scene.h"
#include "Services.h"
#include "StringUtil.h"

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

int IsActorAtLocation(std::string actorName, std::string locationName)
{
	std::string location = Services::Get<CharacterManager>()->GetCharacterLocation(actorName);
	return StringUtil::EqualsIgnoreCase(location, locationName) ? 1 : 0;
}
RegFunc2(IsActorAtLocation, int, string, string);

int IsActorOffstage(std::string actorName)
{
	return Services::Get<CharacterManager>()->IsCharacterOffstage(actorName) ? 1 : 0;
}
RegFunc1(IsActorOffstage, int, string);

int IsCurrentEgo(std::string actorName)
{
	GKActor* ego = GEngine::inst->GetScene()->GetEgo();
	if(ego == nullptr) { return 0; }
	return StringUtil::EqualsIgnoreCase(ego->GetNoun(), actorName) ? 1 : 0;
}
RegFunc1(IsCurrentEgo, int, string);

int WasEgoEverInLocation(std::string locationName)
{
	return 0;
}
RegFunc1(WasEgoEverInLocation, int, string);

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
int GetNounVerbCount(std::string noun, std::string verb)
{
	return 0;
}
RegFunc2(GetNounVerbCount, int, string, string);

int IsCurrentLocation(std::string location)
{
	return 0;
}
RegFunc1(IsCurrentLocation, int, string);

int IsCurrentTime(std::string timeCode)
{
    std::string currentTimeCode = GEngine::inst->GetCurrentTimeCode();
    return StringUtil::EqualsIgnoreCase(currentTimeCode, timeCode);
}
RegFunc1(IsCurrentTime, int, string);

int WasLastLocation(std::string location)
{
	return 0;
}
RegFunc1(WasLastLocation, int, string);

int WasLastTime(std::string timeCode)
{
	return 0;
}
RegFunc1(WasLastTime, int, string);

// INVENTORY
int DoesEgoHaveInvItem(std::string itemName)
{
	return 0;
}
RegFunc1(DoesEgoHaveInvItem, int, string);

int DoesGabeHaveInvItem(std::string itemName)
{
	return 0;
}
RegFunc1(DoesGabeHaveInvItem, int, string);

int DoesGraceHaveInvItem(std::string itemName)
{
    return 0;
}
RegFunc1(DoesGraceHaveInvItem, int, string);

// SCENE
shpvoid SetLocation(std::string location)
{
	GEngine::inst->LoadScene(location);
	return 0;
}
RegFunc1(SetLocation, void, string);
