#include "SheepCompiler.h"

#include <cassert>
#include <fstream>
#include <sstream>

#include "FileSystem.h"
#include "ReportManager.h"
#include "SheepScriptBuilder.h"
#include "StringUtil.h"

SheepScript* SheepCompiler::CompileToAsset(const char* filePath)
{
    assert(filePath != nullptr);

    // Open file stream and pass to istream version.
    std::ifstream inFile(filePath);
    return CompileToAsset(Path::GetFileNameNoExtension(filePath), inFile);
}

SheepScript* SheepCompiler::CompileToAsset(const std::string& name, const std::string& sheep)
{
    // Open string stream and pass to istream version.
    std::stringstream stream(sheep);
    return CompileToAsset(name, stream);
}

SheepScript* SheepCompiler::CompileToAsset(const std::string& name, std::istream& stream)
{
    // Compile the data into the builder, and then create the asset from the builder.
    if(Compile(name, stream))
    {
        return new SheepScript(name, mBuilder);
    }
    return nullptr;
}

bool SheepCompiler::Compile(const std::string& name, std::istream& stream)
{
    // Save name for context in warnings/errors.
    mName = name;

    // Give builder a reference to this compiler (for logging warnings/errors).
    mBuilder.SetCompiler(this);

    // Make sure we can read our stream.
    if(!stream.good() || stream.eof()) { return false; }

    // Create scanner (generated by Flex) that parses text into tokens.
    SheepScanner scanner(&stream);

    // In turn, the parser (generated by Bison) parses tokens into a language grammer.
    Sheep::Parser parser(scanner, *this, mBuilder);

    // Do the parsing! This is where we actually compile the thing.
    int result = parser.parse();
    if(result == 0)
    {
        return true;
    }
    else
    {
        std::cerr << "Failed parsing sheep script with result " << result << std::endl;
        return false;
    }
}

void SheepCompiler::Warning(SheepScriptBuilder* builder, const Sheep::location& location, const std::string& message)
{
    int line = location.begin.line;
    int col = location.begin.column;
    const std::string& section = builder->GetSection();

    // Format and log message.
    std::string reportMsg = StringUtil::Format("GK3 compiler warning at '%s' (line %d, col %d) <%s>\n%s",
                                               mName.c_str(), line, col, section.c_str(),
                                               message.c_str());
    gReportManager.Log("SheepCompilerWarning", reportMsg);
}

void SheepCompiler::Error(SheepScriptBuilder* builder, const Sheep::location& location, const std::string& message)
{
    int line = location.begin.line;
    int col = location.begin.column;
    const std::string& section = builder->GetSection();

    // Format and log message.
    std::string reportMsg = StringUtil::Format("GK3 compiler error at '%s' (line %d, col %d) <%s>\n%s",
                                               mName.c_str(), line, col, section.c_str(),
                                               message.c_str());
    gReportManager.Log("SheepCompilerError", reportMsg);
}
