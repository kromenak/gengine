//
// SheepScript.h
//
// Clark Kromenaker
//
// A compiled sheep script asset.
//
#pragma once
#include "Asset.h"

#include <unordered_map>
#include <vector>

#include "SheepVM.h"

class BinaryReader;
class SheepScriptBuilder;

struct SysImport
{
    std::string name;
    char returnType;
    std::vector<char> argumentTypes;
};

class SheepScript : public Asset
{
public:
    SheepScript(std::string name, char* data, int dataLength);
    SheepScript(std::string name, SheepScriptBuilder& builder);
    
    SysImport* GetSysImport(int index);
    
    std::string GetStringConst(int offset);
    
    std::vector<SheepValue> GetVariables() { return mVariables; }
    
    int GetFunctionOffset(std::string functionName); 
    
    char* GetBytecode() { return mBytecode; }
    int GetBytecodeLength() { return mBytecodeLength; }
    
    void Dump();
    
private:
    std::vector<SysImport> mSysImports;
    
    // String constants, keyed by data offset, since that's how bytecode identifies them.
    std::unordered_map<int, std::string> mStringConsts;
    
    // Represents variable ordering, types, and default values.
    // Bytecode only cares about the index of the variable.
    std::vector<SheepValue> mVariables;
    
    // Maps a function name to it's offset in the bytecode.
    std::unordered_map<std::string, int> mFunctions;
    
    // The bytecode, grabbed directly from the code section.
    // Just pass this to the VM and aaaaawayyyyy we go!
    char* mBytecode = nullptr;
    int mBytecodeLength = 0;
    
    void ParseFromData(char* data, int dataLength);
    void ParseSysImportsSection(BinaryReader& reader);
    void ParseStringConstsSection(BinaryReader& reader);
    void ParseVariablesSection(BinaryReader& reader);
    void ParseFunctionsSection(BinaryReader& reader);
    void ParseCodeSection(BinaryReader& reader);
};
