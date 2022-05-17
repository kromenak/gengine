#include "SheepCompiler.h"

#include <cassert>
#include <fstream>
#include <sstream>

#include "FileSystem.h"
#include "Services.h"
#include "SheepScriptBuilder.h"
#include "StringUtil.h"

SheepCompiler::~SheepCompiler()
{
    delete mScanner;
    mScanner = nullptr;
    
    delete mParser;
    mParser = nullptr;
}

SheepScript* SheepCompiler::Compile(const char* filePath)
{
    assert(filePath != nullptr);
	
	// Grab file name w/o extension as context.
	std::string fileName = Path::GetFileNameNoExtension(filePath);
    std::ifstream inFile(filePath);
    return Compile(fileName, inFile);
}

SheepScript* SheepCompiler::Compile(const std::string& name, const std::string& sheep)
{
    std::stringstream stream(sheep);
    return Compile(name, stream);
}

SheepScript* SheepCompiler::Compile(const std::string& name, std::istream& stream)
{
	// Make sure we can read our stream.
    if(!stream.good() || stream.eof()) { return nullptr; }
	
	// Delete any pre-existing scanner and create a new one.
	// The scanner is used to split the sheep script text into individual tokens.
    delete mScanner;
    try
    {
        mScanner = new SheepScanner(&stream);
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
		SheepScriptBuilder builder(this, name);
        mParser = new Sheep::Parser(*mScanner, *this, builder);
        int result = mParser->parse();
        if(result == 0)
        {
            SheepScript* sheepScript = new SheepScript(name, builder);
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

void SheepCompiler::Warning(SheepScriptBuilder* builder, const Sheep::location& location, const std::string& message)
{
	int line = location.begin.line;
	int col = location.begin.column;
	const std::string& section = builder->GetSection();
	const std::string& name = builder->GetName();
	
	// Format and log message.
	std::string reportMsg = StringUtil::Format("GK3 compiler warning at '%s' (line %d, col %d) <%s>\n%s",
											   name.c_str(), line, col, section.c_str(),
											   message.c_str());
	Services::GetReports()->Log("SheepCompilerWarning", reportMsg);
}

void SheepCompiler::Error(SheepScriptBuilder* builder, const Sheep::location& location, const std::string& message)
{
	int line = location.begin.line;
	int col = location.begin.column;
	const std::string& section = builder->GetSection();
	const std::string& name = builder->GetName();
	
	// Format and log message.
	std::string reportMsg = StringUtil::Format("GK3 compiler error at '%s' (line %d, col %d) <%s>\n%s",
											   name.c_str(), line, col, section.c_str(),
											   message.c_str());
	Services::GetReports()->Log("SheepCompilerError", reportMsg);
}
