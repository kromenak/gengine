#include "SheepSysFunc.h"

#include <iostream>

#include "SheepManager.h"
#include "StringUtil.h"

SysFuncs& GetSysFuncs()
{
    // Since Sheep APIs are now defined across compilation units, global variable init order is undefined.
    // Use static-local to get around that and control init order.
    static SysFuncs sysFuncs;
    return sysFuncs;
}

size_t CalcHashForSysFunc(const SysFuncImport& sysImport)
{
    // This is a variation of Bernstein hash, also called djb2.
    // Original uses values 5381/33 instead of 17/31.
    // It's actually unknown why these values result in a good hash...hope it works out!
    size_t res = 17;
    std::string lowerName = StringUtil::ToLowerCopy(sysImport.name);
    res = res * 31 + std::hash<std::string>()(lowerName);
    res = res * 31 + std::hash<int>()((int)sysImport.argumentTypes.size());
    for(auto& argType : sysImport.argumentTypes)
    {
        res = res * 31 + std::hash<char>()(argType);
    }
    return res;
}

void AddSysFunc(const std::string& name, char retType, std::initializer_list<char> argTypes, bool waitable, bool dev)
{
    SysFunc sysFunc;
    sysFunc.name = name;
    sysFunc.returnType = retType;
    for(auto argType : argTypes)
    {
        sysFunc.argumentTypes.push_back(argType);
    }
    sysFunc.waitable = waitable;
    sysFunc.devOnly = dev;

    SysFuncs& sysFuncs = GetSysFuncs();
    sysFuncs.sysFuncs.push_back(sysFunc);

    // Store a mapping from name and hash to index in the vector of system functions.
    // We can't store references because std::vector can move items around on us during population of the vector.
    sysFuncs.nameToSysFunc[StringUtil::ToLowerCopy(name)] = (int)sysFuncs.sysFuncs.size() - 1;
    sysFuncs.hashToSysFunc[CalcHashForSysFunc(sysFunc)] = (int)sysFuncs.sysFuncs.size() - 1;
}

SysFunc* GetSysFunc(const std::string& name)
{
    auto it = GetSysFuncs().nameToSysFunc.find(name);
    if(it != GetSysFuncs().nameToSysFunc.end())
    {
        return &GetSysFuncs().sysFuncs[it->second];
    }
    return nullptr;
}

SysFunc* GetSysFunc(const SysFuncImport* sysImport)
{
    size_t hash = CalcHashForSysFunc(*sysImport);
    auto it = GetSysFuncs().hashToSysFunc.find(hash);
    if(it != GetSysFuncs().hashToSysFunc.end())
    {
        return &GetSysFuncs().sysFuncs[it->second];
    }
    return nullptr;
}

Value CallSysFunc(const std::string& name)
{
    auto& map0 = GetSysFuncs().map0;
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
    auto& map1 = GetSysFuncs().map1;
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
    auto& map2 = GetSysFuncs().map2;
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
    auto& map3 = GetSysFuncs().map3;
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

Value CallSysFunc(const std::string& name, const Value& x1, const Value& x2, const Value& x3, const Value& x4)
{
    auto& map4 = GetSysFuncs().map4;
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

Value CallSysFunc(const std::string& name, const Value& x1, const Value& x2, const Value& x3, const Value& x4, const Value& x5)
{
    auto& map5 = GetSysFuncs().map5;
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
    auto& map6 = GetSysFuncs().map6;
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

void ExecError()
{
    gSheepManager.FlagExecutionError();
}

const std::string& GetSheepTag()
{
    SheepThread* currentThread = gSheepManager.GetCurrentThread();
    assert(currentThread != nullptr);
    return currentThread->mTag;
}

std::function<void()> AddWait()
{
    SheepThread* currentThread = gSheepManager.GetCurrentThread();
    if(currentThread != nullptr)
    {
        return currentThread->AddWait();
    }
    return nullptr;
}
