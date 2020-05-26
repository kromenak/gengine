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
	// Name of the function.
    std::string name;
	
	// Return type (0 = void, 1 = int, 2 = float, 3 = string).
    char returnType;
	
	// Argument count (size of vector) and types (same as return value).
    std::vector<char> argumentTypes;
	
	// If true, this function can be "waited" upon.
	// If false, it executes and returns immediately.
	bool waitable = false;
	
	// If true, this function can only work in dev builds.
	bool devOnly = false;
	
	//TODO: For in-game help output, we may need to store argument names AND description text.
	// For example, HelpCommand("AddStreamContent") outputs this:
	/*
	----- 'Dump' * 03/16/2019 * 11:39:21 -----
	** [DEBUG] AddStreamContent(string streamName, string content) **
	Adds an additional content type to the stream. Possible values for the 'content' parameter are: 'begin' (report headers), 'content' (report content), 'end' (report footers), 'category' (the category of the report), 'date' (the date the report was made), 'time' (the time the report was made), 'debug' (file/line debug info), 'timeblock' (the current timeblock if there is one), and 'location' (the current game scene location if there is one).
	*/
};

class SheepScript : public Asset
{
public:
    SheepScript(std::string name, char* data, int dataLength);
    SheepScript(const std::string& name, SheepScriptBuilder& builder);
    
    SysImport* GetSysImport(int index);
    
    std::string* GetStringConst(int offset);
    
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
