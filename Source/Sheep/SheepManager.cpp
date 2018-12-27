//
// SheepManager.cpp
//
// Clark Kromenaker
//
#include "SheepManager.h"

#include "Services.h"

SheepScript* SheepManager::Compile(const char *filename)
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

void SheepManager::Execute(std::string assetName, std::string functionName)
{
	SheepScript* script = Services::GetAssets()->LoadSheep(assetName);
	if(script != nullptr)
	{
		Execute(script, functionName);
	}
}

void SheepManager::Execute(SheepScript* script)
{
	mSheepScriptStack.push(script);
	
    SheepVM vm;
    vm.Execute(script);
	
	// VM currently fails when executing multiple scripts (not re-entrant?)
	// Something to look into/fix in the future.
    //mVirtualMachine.Execute(script);
	
	mSheepScriptStack.pop();
}

void SheepManager::Execute(SheepScript* script, std::string functionName)
{
	mSheepScriptStack.push(script);
	
	SheepVM vm;
	vm.Execute(script, functionName);
	//mVirtualMachine.Execute(script, functionName);
	
	mSheepScriptStack.pop();
}

void SheepManager::Execute(std::string functionName)
{
	// This version is assuming a sheep is already executing.
	if(mSheepScriptStack.size() == 0)
	{
		std::cout << "Attempting to execute function " << functionName << ", but no sheep is loaded!" << std::endl;
		return;
	}
	Execute(mSheepScriptStack.top(), functionName);
}

bool SheepManager::Evaluate(SheepScript* script)
{
	mSheepScriptStack.push(script);
	
    SheepVM vm;
	bool result = vm.Evaluate(script);
	//bool result = mVirtualMachine.Evaluate(script);
	
	mSheepScriptStack.pop();
	return result;
}

SheepScript* SheepManager::GetCurrentSheepScript() const
{
	if(mSheepScriptStack.size() > 0)
	{
		return mSheepScriptStack.top();
	}
	return nullptr;
}
