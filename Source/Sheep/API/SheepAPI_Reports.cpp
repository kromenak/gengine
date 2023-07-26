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

    }
    else if(StringUtil::EqualsIgnoreCase(content, "content"))
    {

    }
    else if(StringUtil::EqualsIgnoreCase(content, "end"))
    {

    }
    else if(StringUtil::EqualsIgnoreCase(content, "category"))
    {

    }
    else if(StringUtil::EqualsIgnoreCase(content, "date"))
    {

    }
    else if(StringUtil::EqualsIgnoreCase(content, "time"))
    {

    }
    else if(StringUtil::EqualsIgnoreCase(content, "debug"))
    {

    }
    else if(StringUtil::EqualsIgnoreCase(content, "timeblock"))
    {

    }
    else if(StringUtil::EqualsIgnoreCase(content, "location"))
    {

    }
    else if(StringUtil::EqualsIgnoreCase(content, "machine"))
    {

    }
    else if(StringUtil::EqualsIgnoreCase(content, "user"))
    {

    }
    else if(StringUtil::EqualsIgnoreCase(content, "all"))
    {

    }
    else
    {
        //ERROR: Unknown content!
    }

    //gReportManager.AddStreamContent(streamName, content);
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
    return 0;
}
RegFunc2(RemoveStreamContent, void, string, string, IMMEDIATE, DEV_FUNC);

shpvoid AddStreamOutput(const std::string& streamName, const std::string& output)
{
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
    return 0;
}
RegFunc1(HideReportGraph, void, string, IMMEDIATE, DEV_FUNC);

shpvoid ShowReportGraph(const std::string& graphType)
{
    return 0;
}
RegFunc1(ShowReportGraph, void, string, IMMEDIATE, DEV_FUNC);

shpvoid SetStreamAction(const std::string& streamName, const std::string& action)
{
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