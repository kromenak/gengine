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

std::vector<SysImport> sysFuncs;

Value CallSysFunc(const std::string& name, const Value& x1)
{
    return map1.at(name)(x1);
}

Value CallSysFunc(const std::string& name, const Value& x1, const Value& x2)
{
    return map2.at(name)(x1, x2);
}

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
    if(sysFuncs.size() > 0) { return; }
    AddSysImport("InitEgoPosition", 0, { 3 });
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
