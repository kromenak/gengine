#include "SheepManager.h"

#include "LayerManager.h"
#include "StringUtil.h"

SheepManager gSheepManager;

SheepScript* SheepManager::Compile(const char* filePath)
{
    SheepCompiler compiler;
    return compiler.CompileToAsset(filePath);
}

SheepScript* SheepManager::Compile(const std::string& name, const std::string& sheep)
{
    SheepCompiler compiler;
    return compiler.CompileToAsset(name, sheep);
}

SheepScript* SheepManager::Compile(const std::string& name, std::istream& stream)
{
    SheepCompiler compiler;
    return compiler.CompileToAsset(name, stream);
}

void SheepManager::Execute(SheepScript* script, std::function<void()> finishCallback, const std::string& tag)
{
    // If no tag is provided, fall back on using the current layer's name.
    const std::string& realTag = tag.empty() ? gLayerManager.GetTopLayerName() : tag;

    // Pass to VM for execution.
	mVirtualMachine.Execute(script, finishCallback, realTag);
}

void SheepManager::Execute(SheepScript* script, const std::string& functionName, std::function<void()> finishCallback, const std::string& tag)
{
    // If no tag is provided, fall back on using the current layer's name.
    const std::string& realTag = tag.empty() ? gLayerManager.GetTopLayerName() : tag;

    // Pass to VM for execution.
	mVirtualMachine.Execute(script, functionName, finishCallback, realTag);
}

SheepScript* SheepManager::CompileEval(const std::string& sheep)
{
    // Each eval occurs within a small "husk" consisting of two vars (n/v) and a single function called X$.
    // The passed in Sheep is the body of function X$
    const char* kEvalHusk = "symbols { int n$ = 0; int v$ = 0; } code { X$() %s }";
    std::string fullSheep = StringUtil::Format(kEvalHusk, sheep.c_str());

    SheepCompiler compiler;
    return compiler.CompileToAsset("Case Evaluation", fullSheep);
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
