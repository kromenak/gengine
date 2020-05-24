//
//  SheepCompiler.cpp
//  GEngine
//
//  Created by Clark Kromenaker on 7/24/17.
//
#include "SheepCompiler.h"

#include <cassert>
#include <fstream>
#include <sstream>

#include "Services.h"
#include "SheepAPI.h"
#include "SheepScriptBuilder.h"
#include "StringUtil.h"

SheepCompiler::~SheepCompiler()
{
    delete mScanner;
    mScanner = nullptr;
    
    delete mParser;
    mParser = nullptr;
}

SheepScript* SheepCompiler::Compile(const char *filename)
{
    assert(filename != nullptr);    
    std::ifstream inFile(filename);
    return Compile(inFile);
}

SheepScript* SheepCompiler::Compile(const std::string& sheep)
{
    std::stringstream stream(sheep);
    return Compile(stream);
}

SheepScript* SheepCompiler::Compile(std::istream& stream)
{
	// Make sure we can read our stream.
    if(!stream.good() || stream.eof()) { return nullptr; }
	
	// Delete any pre-existing scanner and create a new one.
	// The scanner is used to split the sheep script text into individual tokens.
    delete mScanner;
    try
    {
        mScanner = new SheepScanner(&stream);
        //DebugOutputTokens(mScanner);
    }
    catch(std::bad_alloc& ba)
    {
        std::cerr << "Failed to allocate scanner: (" << ba.what() << "), exiting!\n";
        return nullptr;
    }
	
	// Delete any pre-existing parser and create a new one.
	// The parser is used to convert tokens into bytecode logic that can be executed.
    delete mParser;
    try
    {
        SheepScriptBuilder builder;
        mParser = new Sheep::Parser(*mScanner, *this, builder);
        int result = mParser->parse();
        if(result == 0)
        {
            SheepScript* sheepScript = new SheepScript("", builder);
            return sheepScript;
        }
        else
        {
            std::cerr << "Failed parsing sheep script with result " << result << std::endl;
            return nullptr;
        }
    }
    catch(std::bad_alloc& ba)
    {
        std::cerr << "Failed to allocate parser: (" << ba.what() << "), exiting!\n";
        return nullptr;
    }
}

void SheepCompiler::Error(const Sheep::location& location, const std::string& message)
{
	int line = location.begin.line;
	int col = location.begin.column;
	std::string section = "Code"; //TODO: I think <Code> is dynamic and changes based on whether section with error is Symbols or Code section.
	std::string sheepContext = "'Console:2'"; //TODO: This should be generated from sheep name.
	
	// Format and log message.
	std::string reportMsg = StringUtil::Format("GK3 compiler error at '%s' (line %d, col %d) <%s>\n%s",
											   sheepContext.c_str(), line, col, section.c_str(),
											   message.c_str());
	Services::GetReports()->Log("SheepCompilerError", reportMsg);
}

void SheepCompiler::DebugOutputTokens(SheepScanner *scanner)
{
    SheepScriptBuilder builder;
    while(true)
    {
        Sheep::Parser::symbol_type yylookahead(scanner->yylex(*scanner, *this, builder));
        if(yylookahead.token() == Sheep::Parser::token::END)
        {
            std::cout << "EOF" << std::endl;
            break;
        }
        else
        {
            std::cout << yylookahead.token() << std::endl;
        }
    }
}
