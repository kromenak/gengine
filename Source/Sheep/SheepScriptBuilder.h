//
// SheepScriptBuilder.h
//
// Clark Kromenaker
//
// Description goes here!
//
#pragma once
#include <vector>
#include <unordered_map>
#include <string>
#include "SheepScript.h"

class SheepScriptBuilder
{
public:
    bool AddSysFuncImport(std::string name);
    
    bool AddStringConst(std::string str);
    
    bool AddIntVariable(std::string name, int defaultValue);
    bool AddFloatVariable(std::string name, float defaultValue);
    bool AddStringVariable(std::string name, std::string defaultValue);
    
    bool AddFunction(std::string functionName);
    
    
    
    void SitNSpin();
    void Yield();
    //CallSysFunctionV
    //CallSysFunctionI
    //CallSysFunctionF
    //CallSysFunctionS
    //Branch
    //BranchGoto
    //BranchIfZero
    void BeginWait();
    void EndWait();
    void ReturnV();
    void Store(std::string varName);
    void Load(std::string varName);
    void PushI(int arg);
    void PushF(float arg);
    void PushS(std::string arg);
    void AddI();
    void AddF();
    void SubtractI();
    void SubtractF();
    void MultiplyI();
    void MultiplyF();
    void DivideI();
    void DivideF();
    void NegateI();
    void NegateF();
    void IsEqualI();
    void IsEqualF();
    void IsNotEqualI();
    void IsNotEqualF();
    void IsGreaterI();
    void IsGreaterF();
    void IsLessI();
    void IsLessF();
    void IsGreaterEqualI();
    void IsGreaterEqualF();
    void IsLessEqualI();
    void IsLessEqualF();
    
    void IToF(int arg);
    void FToI(float arg);
    
    void Modulo();
    
    void And();
    void Or();
    void Not();
    
    void Breakpoint();
    
private:
    std::vector<SysImport> mSysImports;
    
    // String constants, keyed by data offset, since that's how bytecode identifies them.
    std::vector<std::string> mStringConsts;
    
    // Represents variable ordering, types, and default values.
    // Bytecode only cares about the index of the variable.
    std::vector<SheepValue> mVariables;
    std::unordered_map<std::string, int> mVariableIndexByName;
    
    // Maps a function name to it's offset in the bytecode.
    std::unordered_map<std::string, int> mFunctions;
    int mLastFunctionIndex = 0;
    
    // A vector for building the bytecode section.
    std::vector<char> mBytecode;
    
    void AddInstruction(SheepInstruction instr);
    
    void AddIntArg(int arg);
    void AddFloatArg(float arg);
    
    int GetStringConstOffset(std::string stringConst);
};
