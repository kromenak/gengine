#include "SheepAPI_Reports.h"

#include <iomanip>
#include <sstream>

#include "ReportManager.h"
#include "StringUtil.h"

using namespace std;

// TRACING
shpvoid PrintFloat(float value)
{
    gReportManager.Log("SheepScript", std::to_string(value));
    return 0;
}
RegFunc1(PrintFloat, void, float, IMMEDIATE, DEV_FUNC);

shpvoid PrintFloatX(const std::string& category, float value)
{
    gReportManager.Log(category, std::to_string(value));
    return 0;
}
RegFunc2(PrintFloatX, void, string, float, IMMEDIATE, DEV_FUNC);

shpvoid PrintInt(int value)
{
    gReportManager.Log("SheepScript", std::to_string(value));
    return 0;
}
RegFunc1(PrintInt, void, int, IMMEDIATE, DEV_FUNC);

shpvoid PrintIntX(const std::string& category, int value)
{
    gReportManager.Log(category, std::to_string(value));
    return 0;
}
RegFunc2(PrintIntX, void, string, int, IMMEDIATE, DEV_FUNC);

shpvoid PrintIntHex(int value)
{
    std::stringstream ss;
    ss << std::hex << std::setw(2) << std::setfill('0') << std::uppercase << value;
    gReportManager.Log("SheepScript", ss.str());
    return 0;
}
RegFunc1(PrintIntHex, void, int, IMMEDIATE, DEV_FUNC);

shpvoid PrintIntHexX(const std::string& category, int value)
{
    std::stringstream ss;
    ss << std::hex << std::setw(2) << std::setfill('0') << std::uppercase << value;
    gReportManager.Log(category, ss.str());
    return 0;
}
RegFunc2(PrintIntHexX, void, string, int, IMMEDIATE, DEV_FUNC);

shpvoid PrintString(const std::string& string)
{
    gReportManager.Log("SheepScript", string);
    return 0;
}
RegFunc1(PrintString, void, string, IMMEDIATE, DEV_FUNC);

shpvoid PrintStringX(const std::string& category, const std::string& string)
{
    gReportManager.Log(category, string);
    return 0;
}
RegFunc2(PrintStringX, void, string, string, IMMEDIATE, DEV_FUNC);

// REPORTS/STREAMS
shpvoid AddStreamContent(const std::string& streamName, const std::string& content)
{
    if(StringUtil::EqualsIgnoreCase(content, "begin"))
    {
        gReportManager.AddStreamContent(streamName, ReportContent::Begin);
    }
    else if(StringUtil::EqualsIgnoreCase(content, "content"))
    {
        gReportManager.AddStreamContent(streamName, ReportContent::Content);
    }
    else if(StringUtil::EqualsIgnoreCase(content, "end"))
    {
        gReportManager.AddStreamContent(streamName, ReportContent::End);
    }
    else if(StringUtil::EqualsIgnoreCase(content, "category"))
    {
        gReportManager.AddStreamContent(streamName, ReportContent::Category);
    }
    else if(StringUtil::EqualsIgnoreCase(content, "date"))
    {
        gReportManager.AddStreamContent(streamName, ReportContent::Date);
    }
    else if(StringUtil::EqualsIgnoreCase(content, "time"))
    {
        gReportManager.AddStreamContent(streamName, ReportContent::Time);
    }
    else if(StringUtil::EqualsIgnoreCase(content, "timeblock"))
    {
        gReportManager.AddStreamContent(streamName, ReportContent::Timeblock);
    }
    else if(StringUtil::EqualsIgnoreCase(content, "location"))
    {
        gReportManager.AddStreamContent(streamName, ReportContent::Location);
    }
    else if(StringUtil::EqualsIgnoreCase(content, "machine"))
    {
        gReportManager.AddStreamContent(streamName, ReportContent::Machine);
    }
    else if(StringUtil::EqualsIgnoreCase(content, "user"))
    {
        gReportManager.AddStreamContent(streamName, ReportContent::User);
    }
    else if(StringUtil::EqualsIgnoreCase(content, "all"))
    {
        gReportManager.AddStreamContent(streamName, ReportContent::All);
    }
    else
    {
        gReportManager.Log("Warning", StringUtil::Format("Error: %s is not a valid content type. Call HelpCommand(\"AddStreamContent\") for information on possible content types.", content.c_str()));
        ExecError();
    }
    return 0;
}
RegFunc2(AddStreamContent, void, string, string, IMMEDIATE, DEV_FUNC);

shpvoid ClearStreamContent(const std::string& streamName)
{
    gReportManager.ClearStreamContent(streamName);
    return 0;
}
RegFunc1(ClearStreamContent, void, string, IMMEDIATE, DEV_FUNC);

shpvoid RemoveStreamContent(const std::string& streamName, const std::string& content)
{
    if(StringUtil::EqualsIgnoreCase(content, "begin"))
    {
        gReportManager.RemoveStreamContent(streamName, ReportContent::Begin);
    }
    else if(StringUtil::EqualsIgnoreCase(content, "content"))
    {
        gReportManager.RemoveStreamContent(streamName, ReportContent::Content);
    }
    else if(StringUtil::EqualsIgnoreCase(content, "end"))
    {
        gReportManager.RemoveStreamContent(streamName, ReportContent::End);
    }
    else if(StringUtil::EqualsIgnoreCase(content, "category"))
    {
        gReportManager.RemoveStreamContent(streamName, ReportContent::Category);
    }
    else if(StringUtil::EqualsIgnoreCase(content, "date"))
    {
        gReportManager.RemoveStreamContent(streamName, ReportContent::Date);
    }
    else if(StringUtil::EqualsIgnoreCase(content, "time"))
    {
        gReportManager.RemoveStreamContent(streamName, ReportContent::Time);
    }
    else if(StringUtil::EqualsIgnoreCase(content, "timeblock"))
    {
        gReportManager.RemoveStreamContent(streamName, ReportContent::Timeblock);
    }
    else if(StringUtil::EqualsIgnoreCase(content, "location"))
    {
        gReportManager.RemoveStreamContent(streamName, ReportContent::Location);
    }
    else if(StringUtil::EqualsIgnoreCase(content, "machine"))
    {
        gReportManager.RemoveStreamContent(streamName, ReportContent::Machine);
    }
    else if(StringUtil::EqualsIgnoreCase(content, "user"))
    {
        gReportManager.RemoveStreamContent(streamName, ReportContent::User);
    }
    else if(StringUtil::EqualsIgnoreCase(content, "all"))
    {
        gReportManager.RemoveStreamContent(streamName, ReportContent::All);
    }
    else
    {
        gReportManager.Log("Warning", StringUtil::Format("Error: %s is not a valid content type. Call HelpCommand(\"AddStreamContent\") for information on possible content types.", content.c_str()));
        ExecError();
    }
    return 0;
}
RegFunc2(RemoveStreamContent, void, string, string, IMMEDIATE, DEV_FUNC);

shpvoid AddStreamOutput(const std::string& streamName, const std::string& output)
{
    if(StringUtil::EqualsIgnoreCase(output, "console"))
    {
        gReportManager.AddStreamOutput(streamName, ReportOutput::Console);
    }
    else if(StringUtil::EqualsIgnoreCase(output, "debugger"))
    {
        gReportManager.AddStreamOutput(streamName, ReportOutput::Debugger);
    }
    else if(StringUtil::EqualsIgnoreCase(output, "file"))
    {
        gReportManager.AddStreamOutput(streamName, ReportOutput::File);
    }
    else if(StringUtil::EqualsIgnoreCase(output, "memory"))
    {
        gReportManager.AddStreamOutput(streamName, ReportOutput::SharedMemory);
    }
    else if(StringUtil::EqualsIgnoreCase(output, "dialog"))
    {
        gReportManager.AddStreamOutput(streamName, ReportOutput::OSDialog);
    }
    else
    {
        gReportManager.Log("Warning", StringUtil::Format("Error: %s is not a valid output type. Call HelpCommand(\"AddStreamOutput\") for information on possible output types.", output.c_str()));
        ExecError();
    }
    return 0;
}
RegFunc2(AddStreamOutput, void, string, string, IMMEDIATE, DEV_FUNC);

shpvoid ClearStreamOutput(const std::string& streamName)
{
    gReportManager.ClearStreamOutput(streamName);
    return 0;
}
RegFunc1(ClearStreamOutput, void, string, IMMEDIATE, DEV_FUNC);

shpvoid RemoveStreamOutput(const std::string& streamName, const std::string& output)
{
    if(StringUtil::EqualsIgnoreCase(output, "console"))
    {
        gReportManager.RemoveStreamOutput(streamName, ReportOutput::Console);
    }
    else if(StringUtil::EqualsIgnoreCase(output, "debugger"))
    {
        gReportManager.RemoveStreamOutput(streamName, ReportOutput::Debugger);
    }
    else if(StringUtil::EqualsIgnoreCase(output, "file"))
    {
        gReportManager.RemoveStreamOutput(streamName, ReportOutput::File);
    }
    else if(StringUtil::EqualsIgnoreCase(output, "memory"))
    {
        gReportManager.RemoveStreamOutput(streamName, ReportOutput::SharedMemory);
    }
    else if(StringUtil::EqualsIgnoreCase(output, "dialog"))
    {
        gReportManager.RemoveStreamOutput(streamName, ReportOutput::OSDialog);
    }
    else
    {
        gReportManager.Log("Warning", StringUtil::Format("Error: %s is not a valid output type. Call HelpCommand(\"AddStreamOutput\") for information on possible output types.", output.c_str()));
        ExecError();
    }
    return 0;
}
RegFunc2(RemoveStreamOutput, void, string, string, IMMEDIATE, DEV_FUNC);

shpvoid DisableStream(const std::string& streamName)
{
    gReportManager.DisableStream(streamName);
    return 0;
}
RegFunc1(DisableStream, void, string, IMMEDIATE, DEV_FUNC);

shpvoid EnableStream(const std::string& streamName)
{
    gReportManager.EnableStream(streamName);
    return 0;
}
RegFunc1(EnableStream, void, string, IMMEDIATE, DEV_FUNC);

shpvoid HideReportGraph(const std::string& graphType)
{
    // No op - will not implement.
    return 0;
}
RegFunc1(HideReportGraph, void, string, IMMEDIATE, DEV_FUNC);

shpvoid ShowReportGraph(const std::string& graphType)
{
    // No op - will not implement.
    return 0;
}
RegFunc1(ShowReportGraph, void, string, IMMEDIATE, DEV_FUNC);

shpvoid SetStreamAction(const std::string& streamName, const std::string& action)
{
    if(StringUtil::EqualsIgnoreCase(action, "log"))
    {
        gReportManager.SetStreamAction(streamName, ReportAction::Log);
    }
    else if(StringUtil::EqualsIgnoreCase(action, "prompt"))
    {
        gReportManager.SetStreamAction(streamName, ReportAction::Prompt);
    }
    else if(StringUtil::EqualsIgnoreCase(action, "fatal"))
    {
        gReportManager.SetStreamAction(streamName, ReportAction::Fatal);
    }
    else
    {
        gReportManager.Log("Warning", StringUtil::Format("Error: %s is not a valid action type. Call HelpCommand(\"SetStreamAction\") for information on possible action types.", action.c_str()));
        ExecError();
    }
    return 0;
}
RegFunc2(SetStreamAction, void, string, string, IMMEDIATE, DEV_FUNC);

shpvoid SetStreamFilename(const std::string& streamName, const std::string& filename)
{
    gReportManager.SetStreamFilename(streamName, filename);
    return 0;
}
RegFunc2(SetStreamFilename, void, string, string, IMMEDIATE, DEV_FUNC);

shpvoid SetStreamFileTruncate(const std::string& streamName, int truncate)
{
    gReportManager.SetStreamFileTruncate(streamName, truncate != 0);
    return 0;
}
RegFunc2(SetStreamFileTruncate, void, string, int, IMMEDIATE, DEV_FUNC);