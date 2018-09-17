//
// SheepCompiler.h
//
// Clark Kromenaker
//
// Description goes here!
//
#pragma once
#include <istream>
#include <map>
#include <string>

#include "SheepScanner.h"
#include "sheep.tab.hh"

class SheepScript;

class SheepCompiler
{
public:
    SheepCompiler() = default;
    virtual ~SheepCompiler();
    
    SheepScript* Compile(const char* filename);
    SheepScript* Compile(std::string sheep);
    SheepScript* Compile(std::istream& stream);
    
    // Bison requires these to be lowercase.
    void error(const Sheep::location& location, const std::string& message);
    void error(const std::string& message);
    
private:
    SheepScanner* mScanner = nullptr;
    Sheep::Parser* mParser = nullptr;
    
    void DebugOutputTokens(SheepScanner* scanner);
};
