#include "SheepAPI.h"

#include <functional> // for std::hash
#include <sstream> // for int->hex

#include "AssetManager.h"
#include "LayerManager.h"
#include "ReportManager.h"
#include "SheepManager.h"

// Required for macros to work correctly with "string" instead of "std::string".
using namespace std;

shpvoid Call(const std::string& functionName)
{
    // Call desired function on current SheepScript.
    // Gotta grab the current SheepScript from current thread's context.
	SheepThread* currentThread = gSheepManager.GetCurrentThread();
	if(currentThread != nullptr && currentThread->mContext != nullptr)
	{
		SheepScript* sheep = currentThread->mContext->mSheepScript;
		if(sheep != nullptr)
		{
            // Call desired function on this SheepScript.
            if(!functionName.empty() && functionName.back() != '$')
            {
                gSheepManager.Execute(sheep, functionName + "$", AddWait(), GetSheepTag());
            }
            else
            {
                gSheepManager.Execute(sheep, functionName, AddWait(), GetSheepTag());
            }
		}
	}
	return 0;
}
RegFunc1(Call, void, string, WAITABLE, REL_FUNC);

shpvoid CallDefaultSheep(const std::string& fileName)
{
    // Load script and just execute first function in that script.
	SheepScript* script = gAssetManager.LoadSheep(fileName, AssetScope::Scene);
	if(script != nullptr)
	{
        // This new Sheep inherits the tag of the calling Sheep.
		gSheepManager.Execute(script, AddWait(), GetSheepTag());
	}
	return 0;
}
RegFunc1(CallDefaultSheep, void, string, WAITABLE, REL_FUNC);

shpvoid CallSheep(const std::string& fileName, const std::string& functionName)
{
    SheepScript* script = gAssetManager.LoadSheep(fileName, AssetScope::Scene);
    if(script == nullptr)
    {
        gReportManager.Log("Error", StringUtil::Format("Error: unable to find sheep file `%s`", fileName.c_str()));
        ExecError();
        return 0;
    }
    
    // Call desired function on desired SheepScript.
	if(!functionName.empty() && functionName.back() != '$')
	{
        // Make sure function name has the '$' suffix.
	    // Some GK3 data files do this, some don't!
        gSheepManager.Execute(script, functionName + "$", AddWait(), GetSheepTag());
	}
    else
    {
        gSheepManager.Execute(script, functionName, AddWait(), GetSheepTag());
    }
    return 0;
}
RegFunc2(CallSheep, void, string, string, WAITABLE, REL_FUNC);

shpvoid CallGlobal(const std::string& functionName)
{
    // Call desired function on current SheepScript.
    // Gotta grab the current SheepScript from current thread's context.
    SheepThread* currentThread = gSheepManager.GetCurrentThread();
    if(currentThread != nullptr && currentThread->mContext != nullptr)
    {
        SheepScript* sheep = currentThread->mContext->mSheepScript;
        if(sheep != nullptr)
        {
            // Call desired function on this SheepScript.
            if(!functionName.empty() && functionName.back() != '$')
            {
                gSheepManager.Execute(sheep, functionName + "$", AddWait(), gLayerManager.GetBottomLayerName());
            }
            else
            {
                gSheepManager.Execute(sheep, functionName, AddWait(), gLayerManager.GetBottomLayerName());
            }
        }
    }
	return 0;
}
RegFunc1(CallGlobal, void, string, WAITABLE, REL_FUNC);

shpvoid CallGlobalSheep(const std::string& sheepFileName, const std::string& functionName)
{
    SheepScript* script = gAssetManager.LoadSheep(sheepFileName, AssetScope::Scene);
    if(script == nullptr)
    {
        ExecError();
        return 0;
    }

    // Call desired function on desired SheepScript.
    if(!functionName.empty() && functionName.back() != '$')
    {
        // Make sure function name has the '$' suffix.
        // Some GK3 data files do this, some don't!
        gSheepManager.Execute(script, functionName + "$", AddWait(), gLayerManager.GetBottomLayerName());
    }
    else
    {
        gSheepManager.Execute(script, functionName, AddWait(), gLayerManager.GetBottomLayerName());
    }
    return 0;
}
RegFunc2(CallGlobalSheep, void, string, string, WAITABLE, REL_FUNC);

std::string GetCurrentSheepFunction()
{
	SheepThread* currentThread = gSheepManager.GetCurrentThread();
	return currentThread != nullptr ? currentThread->mFunctionName : "";
}
RegFunc0(GetCurrentSheepFunction, string, IMMEDIATE, REL_FUNC);

std::string GetCurrentSheepName()
{
	SheepThread* currentThread = gSheepManager.GetCurrentThread();
	if(currentThread != nullptr && currentThread->mContext != nullptr)
	{
		// Specifically, return name with no extension.
		return currentThread->mContext->mSheepScript->GetNameNoExtension();
	}
	return "";
}
RegFunc0(GetCurrentSheepName, string, IMMEDIATE, REL_FUNC);

shpvoid SetGlobalSheep()
{
    SheepThread* currentThread = gSheepManager.GetCurrentThread();
    if(currentThread != nullptr)
    {
        // Attach current sheep to global layer.
        // Means sheep stays active even when adding new game layers or changing scenes.
        currentThread->mTag = gLayerManager.GetBottomLayerName();
    }
	return 0;
}
RegFunc0(SetGlobalSheep, void, IMMEDIATE, REL_FUNC);

shpvoid SetTopSheep()
{
    SheepThread* currentThread = gSheepManager.GetCurrentThread();
    if(currentThread != nullptr)
    {
        // Attach current sheep to top layer.
        // Sheep will stop on scene change.
        currentThread->mTag = gLayerManager.GetTopLayerName();
    }
	return 0;
}
RegFunc0(SetTopSheep, void, IMMEDIATE, REL_FUNC);
 
//NukeAllSheep
//NukeSheep

shpvoid DumpSheepEngine()
{
    //TODO;
    return 0;
}
RegFunc0(DumpSheepEngine, void, IMMEDIATE, DEV_FUNC);