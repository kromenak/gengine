//
// SheepAPI.cpp
//
// Clark Kromenaker
//
#include "SheepAPI.h"
#include "GEngine.h"
#include "Stage.h"

std::map<std::string, Value (*)(const Value&)> map1;
std::map<std::string, Value (*)(const Value&, const Value&)> map2;

Value CallSysFunc(const std::string& name, const Value& x1)
{
    return map1.at(name)(x1);
}

Value CallSysFunc(const std::string& name, const Value& x1, const Value& x2)
{
    return map2.at(name)(x1, x2);
}

shpvoid CallSheep(std::string fileName, std::string functionName)
{
    return 0;
}

shpvoid InitEgoPosition(std::string positionName)
{
    GEngine::inst->GetStage()->InitEgoPosition(positionName);
    return 0;
}

RegFunc1(InitEgoPosition, std::string);
