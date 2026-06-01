#include "SheepManager.h"

#include "LayerManager.h"
#include "PersistState.h"
#include "ReportManager.h"
#include "StringUtil.h"

SheepManager gSheepManager;

void SheepManager::Init()
{
    // A stream for Sheep compiler info logs.
    ReportStream& sheepCompilerInfo = gReportManager.GetReportStream("SheepCompilerInfo");
    sheepCompilerInfo.AddOutput(ReportOutput::Debugger | ReportOutput::SharedMemory | ReportOutput::Console);
    sheepCompilerInfo.AddContent(ReportContent::AllButDateMachineUser);
    sheepCompilerInfo.Disable(); // disabled by default

    // A stream for Sheep compiler warning logs.
    ReportStream& sheepCompilerWarning = gReportManager.GetReportStream("SheepCompilerWarning");
    sheepCompilerWarning.AddOutput(ReportOutput::Debugger | ReportOutput::SharedMemory | ReportOutput::Console);
    sheepCompilerWarning.AddContent(ReportContent::AllButDateMachineUser);

    // A stream for Sheep compiler error logs.
    ReportStream& sheepCompilerError = gReportManager.GetReportStream("SheepCompilerError");
    sheepCompilerError.AddOutput(ReportOutput::Debugger | ReportOutput::SharedMemory | ReportOutput::Console);
    sheepCompilerError.AddContent(ReportContent::AllButDateMachineUser);

    // A stream for Sheep compiler fatal logs.
    ReportStream& sheepCompilerFatal = gReportManager.GetReportStream("SheepCompilerFatal");
    sheepCompilerFatal.AddOutput(ReportOutput::Debugger | ReportOutput::SharedMemory | ReportOutput::Console);
    sheepCompilerFatal.AddContent(ReportContent::AllButDateMachineUser);

    // A stream for sheep engine data.
    ReportStream& sheepEngine = gReportManager.GetReportStream("SheepEngine");
    sheepEngine.AddOutput(ReportOutput::Debugger | ReportOutput::SharedMemory | ReportOutput::Console);
    sheepEngine.AddContent(ReportContent::AllButDateMachineUser);

    // A stream for Sheep virtual machine execution logs.
    ReportStream& sheepMachine = gReportManager.GetReportStream("SheepMachine");
    sheepMachine.AddOutput(ReportOutput::Debugger | ReportOutput::SharedMemory | ReportOutput::Console);
    sheepMachine.AddContent(ReportContent::AllButDateMachineUser);
    sheepMachine.Disable(); // disabled by default

    // A stream for logs coming from executing SheepScripts (i.e. from PrintString/PrintFloat/etc calls).
    ReportStream& sheepScript = gReportManager.GetReportStream("SheepScript");
    sheepScript.AddOutput(ReportOutput::Debugger | ReportOutput::SharedMemory | ReportOutput::Console);
    sheepScript.AddContent(ReportContent::AllButDateMachineUser);

    // A stream for Sheep system call logging.
    ReportStream& sheepSysCalls = gReportManager.GetReportStream("SheepSysCalls");
    sheepSysCalls.AddOutput(ReportOutput::Debugger | ReportOutput::SharedMemory | ReportOutput::Console);
    sheepSysCalls.AddContent(ReportContent::Content);
}

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

SheepThreadId SheepManager::Execute(SheepScript* script, std::function<void()> finishCallback, const std::string& tag)
{
    // If no tag is provided, fall back on using the current layer's name.
    const std::string& realTag = tag.empty() ? gLayerManager.GetTopLayerName() : tag;

    // Pass to VM for execution.
    return mVirtualMachine.Execute(script, finishCallback, realTag);
}

SheepThreadId SheepManager::Execute(SheepScript* script, const std::string& functionName, std::function<void()> finishCallback, const std::string& tag)
{
    // If no tag is provided, fall back on using the current layer's name.
    const std::string& realTag = tag.empty() ? gLayerManager.GetTopLayerName() : tag;

    // Pass to VM for execution.
    return mVirtualMachine.Execute(script, functionName, finishCallback, realTag);
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

void SheepManager::OnPersist(PersistState& ps)
{
    // This was added in save version 4, so only do it in that case.
    if(ps.GetFormatVersionNumber() >= 4)
    {
        mVirtualMachine.OnPersist(ps);
    }
}