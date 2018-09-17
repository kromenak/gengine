//
//  SheepScript.cpp
//  GEngine
//
//  Created by Clark Kromenaker on 7/23/17.
//
#include "SheepScript.h"

#include <iostream>

#include "BinaryReader.h"
#include "SheepScriptBuilder.h"

SheepScript::SheepScript(std::string name, char* data, int dataLength) : Asset(name)
{
    ParseFromData(data, dataLength);
}

SheepScript::SheepScript(std::string name, SheepScriptBuilder& builder) : Asset(name)
{
    // Just copy these directly.
    mSysImports = builder.GetSysImports();
    mStringConsts = builder.GetStringConsts();
    mVariables = builder.GetVariables();
    mFunctions = builder.GetFunctions();
    
    // Bytecode needs to convert from std::vector to byte array.
    std::vector<char> bytecodeVec = builder.GetBytecode();
    mBytecodeLength = (int)bytecodeVec.size();
    mBytecode = new char[mBytecodeLength];
    std::copy(bytecodeVec.begin(), bytecodeVec.end(), mBytecode);
}

SysImport* SheepScript::GetSysImport(int index)
{
    if(index < 0 || index >= mSysImports.size()) { return nullptr; }
    return &mSysImports[index];
}

std::string SheepScript::GetStringConst(int offset)
{
    auto it = mStringConsts.find(offset);
    if(it != mStringConsts.end())
    {
        return it->second;
    }
    return std::string();
}

int SheepScript::GetFunctionOffset(std::string functionName)
{
    auto it = mFunctions.find(functionName);
    if(it != mFunctions.end())
    {
        return it->second;
    }
    return -1;
}

void SheepScript::Dump()
{
    std::cout << "Dumping sheep " << mName << std::endl << std::endl;
    std::cout << "--------------------------------------------------------------------------" << std::endl;
    std::cout << "Component   : GK3Sheep" << std::endl;
    std::cout << "--------------------------------------------------------------------------" << std::endl;
}

void SheepScript::ParseFromData(char *data, int dataLength)
{
    BinaryReader reader(data, dataLength);
    
    // First 8 bytes: file identifier "GK3Sheep".
    std::string identifier = reader.ReadString(8);
    if(identifier != "GK3Sheep")
    {
        std::cout << "Not valid GK3Sheep data!" << std::endl;
        return;
    }
    
    // 4 bytes: maybe a format version number?
    reader.Skip(4);
    
    // 4 bytes: size of header
    int headerSize = reader.ReadInt();
    
    // 4 bytes: size of header, duped
    // 4 bytes: size of file contents, minus 44 byte header
    reader.Skip(8);
    
    int dataCount = reader.ReadInt();
    int dataOffsets[dataCount];
    for(int i = 0; i < dataCount; i++)
    {
        dataOffsets[i] = reader.ReadInt();
    }
    
    for(int i = 0; i < dataCount; i++)
    {
        int offset = dataOffsets[i] + headerSize;
        reader.Seek(offset);
        
        std::string identifier = reader.ReadString(12);
        if(identifier == "SysImports")
        {
            ParseSysImportsSection(reader);
        }
        else if(identifier == "StringConsts")
        {
            ParseStringConstsSection(reader);
        }
        else if(identifier == "Variables")
        {
            ParseVariablesSection(reader);
        }
        else if(identifier == "Functions")
        {
            ParseFunctionsSection(reader);
        }
        else if(identifier == "Code")
        {
            ParseCodeSection(reader);
        }
        else
        {
            std::cout << "Unknown component: " << identifier << std::endl;
        }
    }
}

void SheepScript::ParseSysImportsSection(BinaryReader& reader)
{
    // Already read the identifier.
    // Don't need header size (x2).
    // Don't need byte size of all imports.
    reader.Skip(12);
    
    int functionCount = reader.ReadInt();
    
    // Don't really need offset values for functions.
    reader.Skip(4 * functionCount);
    
    for(int i = 0; i < functionCount; i++)
    {
        SysImport import;
        
        // Read in name from length.
        // Length is always one more, due to null terminator.
        short nameLength = reader.ReadShort();
        import.name = reader.ReadString(nameLength + 1);;
        
        import.returnType = reader.ReadByte();
        
        char argumentCount = reader.ReadByte();
        for(int i = 0; i < argumentCount; i++)
        {
            import.argumentTypes.push_back(reader.ReadByte());
        }
        
        mSysImports.push_back(import);
    }
}

void SheepScript::ParseStringConstsSection(BinaryReader& reader)
{
    // Already read the identifier.
    // Don't need header size (x2).
    reader.Skip(8);
    
    int contentSize = reader.ReadInt();
    int stringCount = reader.ReadInt();
    int stringOffsets[stringCount];
    for(int i = 0; i < stringCount; i++)
    {
        stringOffsets[i] = reader.ReadInt();
    }
    
    int dataBaseOffset = reader.GetPosition();
    for(int i = 0; i < stringCount; i++)
    {
        int startOffset = dataBaseOffset + stringOffsets[i];
        int endOffset;
        if(i < stringCount - 1)
        {
            endOffset = dataBaseOffset + stringOffsets[i + 1];
        }
        else
        {
            endOffset = dataBaseOffset + contentSize;
        }
        std::string str = reader.ReadString(endOffset - startOffset);
        mStringConsts[startOffset - dataBaseOffset] = str;
    }
}

void SheepScript::ParseVariablesSection(BinaryReader& reader)
{
    // Already read the identifier.
    // Don't need header size (x2).
    // Don't need byte size of all variables.
    reader.Skip(12);
    
    int variableCount = reader.ReadInt();
    
    // Don't really need offset values for variables.
    reader.Skip(4 * variableCount);
    
    // Read in each variable.
    for(int i = 0; i < variableCount; i++)
    {
        SheepValue value;
        
        // Read in name from length.
        // Length is always one more, due to null terminator.
        short nameLength = reader.ReadShort();
        std::string name = reader.ReadString(nameLength + 1);
        
        // Type is either int, float, or string.
        int type = reader.ReadInt();
        if(type == 1)
        {
            value.type = SheepValueType::Int;
            value.intValue = reader.ReadInt();
        }
        else if(type == 2)
        {
            value.type = SheepValueType::Float;
            value.floatValue = reader.ReadFloat();
        }
        else if(type == 3)
        {
            value.type = SheepValueType::String;
            reader.ReadInt();
            value.stringValue = nullptr;
        }
        else
        {
            std::cout << "Unknown type: " << type << std::endl;
        }
        mVariables.push_back(value);
    }
}

void SheepScript::ParseFunctionsSection(BinaryReader& reader)
{
    // Already read the identifier.
    // Don't need header size (x2).
    // Don't need byte size of all functions.
    reader.Skip(12);
    
    int functionCount = reader.ReadInt();
    
    // Don't really need offset values for functions.
    reader.Skip(4 * functionCount);
    
    for(int i = 0; i < functionCount; i++)
    {
        // Read in name from length.
        // Length is always one more, due to null terminator.
        short nameLength = reader.ReadShort();
        std::string name = reader.ReadString(nameLength + 1);
        
        reader.Skip(2);
        
        int codeOffset = reader.ReadInt();
        mFunctions[name] = codeOffset;
    }
}

void SheepScript::ParseCodeSection(BinaryReader& reader)
{
    // Already read the identifier.
    // Don't need header sizes.
    reader.Skip(8);
    
    // Get size in bytes of code section after header.
    mBytecodeLength = reader.ReadInt();
    
    // Next is number of code sections - should always be one.
    int codeContentCount = reader.ReadInt();
    if(codeContentCount != 1)
    {
        std::cout << "Expected one!" << std::endl;
        return;
    }
    
    // Next is the offset to each code content. But since
    // there's only one, this will always be zero.
    reader.ReadInt();
    
    // The rest is just bytecode!
    mBytecode = new char[mBytecodeLength];
    reader.Read(mBytecode, mBytecodeLength);
}
