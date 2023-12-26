//
// Clark Kromenaker
//
// A compiled sheep script asset.
//
#pragma once
#include "Asset.h"

#include <string>
#include <unordered_map>
#include <vector>

#include "SheepSysFunc.h"
#include "SheepVM.h"
#include "StringUtil.h"

class BinaryReader;
class SheepScriptBuilder;

class SheepScript : public Asset
{
public:
    static bool IsSheepDataCompiled(uint8_t* data, uint32_t dataLength);

    SheepScript(const std::string& name, AssetScope scope) : Asset(name, scope) { }
    SheepScript(const std::string& name, SheepScriptBuilder& builder);
    ~SheepScript() override;

    void Load(uint8_t* data, uint32_t dataLength);
    void Load(const SheepScriptBuilder& builder);

    SysFuncImport* GetSysImport(int index);

    std::string* GetStringConst(int offset);

    std::vector<SheepValue> GetVariables() { return mVariables; }

    int GetFunctionOffset(const std::string& functionName);
    const std::string* GetFunctionAtOffset(int offset) const;

    char* GetBytecode() { return mBytecode; }
    int GetBytecodeLength() const { return mBytecodeLength; }

    void Dump();
    void Decompile();
    void Decompile(const std::string& filePath);

private:
    // List of SysFuncs this script uses.
    std::vector<SysFuncImport> mSysImports;

    // String constants, keyed by data offset, since that's how bytecode identifies them.
    std::unordered_map<int, std::string> mStringConsts;

    // Represents variable ordering, types, and default values.
    // Bytecode only cares about the index of the variable.
    std::vector<SheepValue> mVariables;

    // Maps a function name to it's offset in the bytecode.
	std::string_map_ci<int> mFunctions;

    // The bytecode, grabbed directly from the code section.
    // Just pass this to the VM and aaaaawayyyyy we go!
    char* mBytecode = nullptr;
    int mBytecodeLength = 0;

    void ParseFromData(uint8_t* data, uint32_t dataLength);
    void ParseSysImportsSection(BinaryReader& reader);
    void ParseStringConstsSection(BinaryReader& reader);
    void ParseVariablesSection(BinaryReader& reader);
    void ParseFunctionsSection(BinaryReader& reader);
    void ParseCodeSection(BinaryReader& reader);
};
