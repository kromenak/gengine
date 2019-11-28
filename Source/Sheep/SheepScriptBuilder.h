//
// SheepScriptBuilder.h
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

class SheepScriptBuilder
{
public:
    SheepScriptBuilder();
	~SheepScriptBuilder();
    
    void AddStringConst(std::string str);
    void AddIntVariable(std::string name, int defaultValue);
    void AddFloatVariable(std::string name, float defaultValue);
    void AddStringVariable(std::string name, std::string defaultValue);
    
    void StartFunction(std::string functionName);
    void EndFunction(std::string functionName);
    
    void AddGoto(std::string labelName);
	void BranchGoto(std::string labelName);
    
    void SitnSpin();
    void Yield();
	
	void AddToSysFuncArgCount() { ++mSysFuncArgCount; }
    SheepValueType CallSysFunc(std::string sysFuncName);
	
	void BeginIfElseBlock();
	void EndIfElseBlock();
	
	void BeginIfBlock();
	void EndIfBlock();
	
	void BeginElseBlock();
	void EndElseBlock();
	
    void BeginWait();
    void EndWait();
	
    void ReturnV();
	
    void Store(std::string varName);
    SheepValueType Load(std::string varName);
	
    void PushI(int arg);
    void PushF(float arg);
    void PushS(std::string arg);
    
    SheepValueType Add(SheepValue val1, SheepValue val2);
    SheepValueType Subtract(SheepValue val1, SheepValue val2);
    SheepValueType Multiply(SheepValue val1, SheepValue val2);
    SheepValueType Divide(SheepValue val1, SheepValue val2);
    void Negate(SheepValue val);
	
    SheepValueType IsEqual(SheepValue val1, SheepValue val2);
    SheepValueType IsNotEqual(SheepValue val1, SheepValue val2);
    SheepValueType IsGreater(SheepValue val1, SheepValue val2);
    SheepValueType IsLess(SheepValue val1, SheepValue val2);
    SheepValueType IsGreaterEqual(SheepValue val1, SheepValue val2);
    SheepValueType IsLessEqual(SheepValue val1, SheepValue val2);
    
    void IToF(int index);
    void FToI(int index);
    
    void Modulo(SheepValue val1, SheepValue val2);
    
    void And(SheepValue val1, SheepValue val2);
    void Or(SheepValue val1, SheepValue val2);
    void Not();
    
    void Breakpoint();
	
	// Parser code can call this function after calling an above function to see if an internal compiler error occurred.
	// Usually, the compiler will want to abort if this returns true.
	bool CheckError(const Sheep::Parser::location_type& loc, Sheep::Parser& parser) const;
    
	// After "building" the script's bytecode and tables, these functions are used to access that data.
    std::vector<SysImport> GetSysImports() { return mSysImports; }
    std::unordered_map<int, std::string> GetStringConsts() { return mStringConstsByOffset; }
    std::vector<SheepValue> GetVariables() { return mVariables; }
    std::unordered_map<std::string, int> GetFunctions() { return mFunctions; }
    std::vector<char> GetBytecode() { return mBytecode; }
    
private:
    // Definition for any system functions used.
    std::vector<SysImport> mSysImports;
	
	// When building system function calls, its useful to keep track of the arg count given by the user.
	// If the arg count doesn't match the expected number of arguments, we can generate a compiler error.
	int mSysFuncArgCount = 0;
    
    // String constants, keyed by data offset, since that's how bytecode identifies them.
    std::vector<std::string> mStringConsts;
    int mStringConstsOffset = 0;
    std::unordered_map<int, std::string> mStringConstsByOffset;
    
    // Represents variable ordering, types, and default values.
    // Bytecode only cares about the index of the variable. But we maintain a map by name to detect duplicates.
    std::vector<SheepValue> mVariables;
    std::unordered_map<std::string, int> mVariableIndexByName;
    
    // Maps a function name to it's offset in the bytecode.
    std::unordered_map<std::string, int> mFunctions;
    
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
	
	// In case of error, this flag will be set.
	std::string mErrorMessage;
    
    void AddInstruction(SheepInstruction instr);
    
    void AddIntArg(int arg);
    void AddFloatArg(float arg);
    
    int GetStringConstOffset(std::string stringConst);
	
	void SetError(const std::string& message);
};
