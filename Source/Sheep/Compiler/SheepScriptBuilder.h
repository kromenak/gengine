//
// Clark Kromenaker
//
// When parsing Sheep tokens during compilation, this builder is used to
// maintain state/scope and construct the final bytecode stream.
//
#pragma once
#include <string>
#include <unordered_map>
#include <vector>

#include "SheepScript.h"
#include "sheep.tab.hh"
#include "StringUtil.h"

class SheepCompiler;

// A "location" in a Sheep script (i.e. line/column numbers).
using Location = Sheep::Parser::location_type;

class SheepScriptBuilder
{
public:
    SheepScriptBuilder();
	~SheepScriptBuilder();

    void SetCompiler(SheepCompiler* compiler) { mCompiler = compiler; }

	void BeginSymbols() { mSection = "Symbols"; }
	
    void AddStringConst(std::string str);
    void AddIntVariable(std::string name, int defaultValue);
    void AddFloatVariable(std::string name, float defaultValue);
    void AddStringVariable(std::string name, std::string defaultValue);
    
	void BeginCode() { mSection = "Code"; }
	
    void StartFunction(std::string functionName);
    void EndFunction(std::string functionName);
    
    void AddGoto(std::string labelName, const Location& loc);
	void BranchGoto(std::string labelName);
    
    void SitnSpin();
    void Yield();
	
	void AddSysFuncArg(SheepValue arg, const Location& loc);
    SheepValueType CallSysFunc(std::string sysFuncName, const Location& loc);
	
	void BeginIfElseBlock();
	void EndIfElseBlock();
	
	void BeginIfBlock();
	void EndIfBlock();
	
	void BeginElseBlock();
	void EndElseBlock();
	
    void BeginWait();
    void EndWait();
	
    void ReturnV();
	
    void Store(std::string varName, const Location& loc);
    SheepValueType Load(std::string varName, const Location& loc);
	
    void PushI(int arg);
    void PushF(float arg);
    void PushS(std::string arg);
    
    SheepValueType Add(SheepValue val1, SheepValue val2, const Location& loc);
    SheepValueType Subtract(SheepValue val1, SheepValue val2, const Location& loc);
    SheepValueType Multiply(SheepValue val1, SheepValue val2, const Location& loc);
    SheepValueType Divide(SheepValue val1, SheepValue val2, const Location& loc);
    void Negate(SheepValue val, const Location& loc);
	
    SheepValueType IsEqual(SheepValue val1, SheepValue val2, const Location& loc);
    SheepValueType IsNotEqual(SheepValue val1, SheepValue val2, const Location& loc);
    SheepValueType IsGreater(SheepValue val1, SheepValue val2, const Location& loc);
    SheepValueType IsLess(SheepValue val1, SheepValue val2, const Location& loc);
    SheepValueType IsGreaterEqual(SheepValue val1, SheepValue val2, const Location& loc);
    SheepValueType IsLessEqual(SheepValue val1, SheepValue val2, const Location& loc);
    
    void IToF(int index);
    void FToI(int index);
    
    void Modulo(SheepValue val1, SheepValue val2, const Location& loc);
    
    void And(SheepValue val1, SheepValue val2, const Location& loc);
    void Or(SheepValue val1, SheepValue val2, const Location& loc);
    void Not();
    
    void Breakpoint();
	
	const std::string& GetSection() const { return mSection; }
	
	// After "building" the script's bytecode and tables, these functions are used to access that data.
    const std::vector<SysFuncImport>& GetSysImports() const { return mSysImports; }
    const std::unordered_map<int, std::string>& GetStringConsts() const { return mStringConstsByOffset; }
    const std::vector<SheepValue>& GetVariables() const { return mVariables; }
    const std::string_map_ci<int>& GetFunctions() const { return mFunctions; }
    const std::vector<char>& GetBytecode() const { return mBytecode; }
    
private:
	// Reference back to the compiler.
	SheepCompiler* mCompiler = nullptr;
	
    // Definition for any system functions used.
    std::vector<SysFuncImport> mSysImports;
	
	// When building system function calls, its useful to keep track of the args given by the user.
	// If the arg count doesn't match the expected number of arguments, we can generate a compiler error.
	// Also, if arg types don't match (and aren't compatible/convertable), that's also a compiler error or warning.
	std::vector<SheepValue> mSysFuncArgs;
    
    // String constants, keyed by data offset, since that's how bytecode identifies them.
    std::vector<std::string> mStringConsts;
    int mStringConstsOffset = 0;
    std::unordered_map<int, std::string> mStringConstsByOffset;
    
    // Represents variable ordering, types, and default values.
    // Bytecode only cares about the index of the variable. But we maintain a map by name to detect duplicates.
    std::vector<SheepValue> mVariables;
    std::unordered_map<std::string, int> mVariableIndexByName;
    
    // Maps a function name to it's offset in the bytecode.
    std::string_map_ci<int> mFunctions;
    
    // For gotos, a map of label to bytecode offset.
    std::unordered_map<std::string, int> mGotoLabelsToOffsets;
    std::unordered_map<std::string, std::vector<int>> mGotoLabelsToBeHookedUp;
	
	// For if/else blocks, we need to track what block we're in and how many nested blocks exist as we parse/compile.
	struct OpenIfBlock
	{
		// Each if statement needs to know where to branch to if the if condition evaluates to false.
		// We store the offset where we need to fill in that address here.
		int branchIfZeroAddressOffset = -1;
		
		// At the end of each if block, we need to put a branch to the point past the end of the if/else block.
		// E.g. after first "if" block, we don't want to execute 2nd/3rd/etc "else if" blocks - must branch!
		// Here, we store all the offsets where we need to fill in the end of the if/else block address.
		// We can then circle back and fill in the missing values once we know where the if/else block ends.
		std::vector<int> branchAddressOffsets;
	};
	std::vector<OpenIfBlock> mOpenIfBlocks;
    
    // A vector for building the bytecode section.
    std::vector<char> mBytecode;
	
	// Section currently being compiled.
	std::string mSection = "Code";
	
    void AddInstruction(SheepInstruction instr);
    
    void AddIntArg(int arg);
    void AddFloatArg(float arg);
    
    int GetStringConstOffset(std::string stringConst);
	
	void LogWarning(const Location& loc, const std::string& message);
	void LogError(const Location& loc, const std::string& message);
};
