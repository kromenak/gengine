//
// SheepManager.cpp
//
// Clark Kromenaker
//
#include "SheepManager.h"

#include "Services.h"

SheepScript* SheepManager::Compile(const char* filename)
{
    return mCompiler.Compile(filename);
}

SheepScript* SheepManager::Compile(std::string sheep)
{
    return mCompiler.Compile(sheep);
}

SheepScript* SheepManager::Compile(std::istream &stream)
{
    return mCompiler.Compile(stream);
}

void SheepManager::Execute(std::string sheepName, std::string functionName)
{
	SheepScript* script = Services::GetAssets()->LoadSheep(sheepName);
	if(script != nullptr)
	{
		Execute(script, functionName, nullptr);
	}
}

void SheepManager::Execute(std::string sheepName, std::string functionName, std::function<void()> finishCallback)
{
	SheepScript* script = Services::GetAssets()->LoadSheep(sheepName);
	if(script != nullptr)
	{
		Execute(script, functionName, finishCallback);
	}
}

void SheepManager::Execute(SheepScript* script)
{
    mVirtualMachine.Execute(script);
}

void SheepManager::Execute(SheepScript* script, std::string functionName, std::function<void()> finishCallback)
{
	mVirtualMachine.Execute(script, functionName, finishCallback);
}

bool SheepManager::Evaluate(SheepScript* script)
{
	return mVirtualMachine.Evaluate(script);
}
