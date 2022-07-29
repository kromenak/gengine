#include "SheepManager.h"

#include "Services.h"
#include "StringUtil.h"

SheepScript* SheepManager::Compile(const char* filePath)
{
    return mCompiler.Compile(filePath);
}

SheepScript* SheepManager::Compile(const std::string& name, const std::string& sheep)
{
    return mCompiler.Compile(name, sheep);
}

SheepScript* SheepManager::Compile(const std::string& name, std::istream& stream)
{
    return mCompiler.Compile(name, stream);
}

SheepScript* SheepManager::CompileEval(const std::string& sheep)
{
    // Each eval occurs within a small "husk" consisting of two vars (n/v) and a single function called X$.
    // The passed in Sheep is the body of function X$
    const char* kEvalHusk = "symbols { int n$ = 0; int v$ = 0; } code { X$() %s }";
	std::string fullSheep = StringUtil::Format(kEvalHusk, sheep.c_str());
	return mCompiler.Compile("Case Evaluation", fullSheep);
}

void SheepManager::Execute(const std::string& sheepName, const std::string& functionName, std::function<void()> finishCallback)
{
	SheepScript* script = Services::GetAssets()->LoadSheep(sheepName);
	if(script != nullptr)
	{
		Execute(script, functionName, finishCallback);
	}
}

void SheepManager::Execute(SheepScript* script, std::function<void()> finishCallback)
{
	mVirtualMachine.Execute(script, finishCallback);
}

void SheepManager::Execute(SheepScript* script, const std::string& functionName, std::function<void()> finishCallback)
{
	mVirtualMachine.Execute(script, functionName, finishCallback);
}

bool SheepManager::Evaluate(SheepScript* script)
{
	return Evaluate(script, 0, 0);
}

bool SheepManager::Evaluate(SheepScript* script, int n, int v)
{
	// No script automatically equates to "true".
	if(script == nullptr) { return true; }
	
	// Otherwise, do it "for real."
	return mVirtualMachine.Evaluate(script, n, v);
}
