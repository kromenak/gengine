//
// Clark Kromenaker
//
// Sheep SysFuncs related to the reports system and logging.
//
#pragma once
#include "SheepSysFunc.h"

// TRACING
shpvoid PrintFloat(float value);
shpvoid PrintFloatX(const std::string& category, float value);

shpvoid PrintInt(int value);
shpvoid PrintIntX(const std::string& category, int value);

shpvoid PrintIntHex(int value);
shpvoid PrintIntHexX(const std::string& category, int value);

shpvoid PrintString(const std::string& message);
shpvoid PrintStringX(const std::string& category, const std::string& message);

// REPORTS/STREAMS
shpvoid AddStreamContent(const std::string& streamName, const std::string& content);
shpvoid ClearStreamContent(const std::string& streamName);
shpvoid RemoveStreamContent(const std::string& streamName, const std::string& content);

shpvoid AddStreamOutput(const std::string& streamName, const std::string& output);
shpvoid ClearStreamOutput(const std::string& streamName);
shpvoid RemoveStreamOutput(const std::string& streamName, const std::string& output);

shpvoid DisableStream(const std::string& stream);
shpvoid EnableStream(const std::string& stream);

shpvoid HideReportGraph(const std::string& graphType);
shpvoid ShowReportGraph(const std::string& graphType);

shpvoid SetStreamAction(const std::string& streamName, const std::string& action);
shpvoid SetStreamFilename(const std::string& streamName, const std::string& filename);
shpvoid SetStreamFileTruncate(const std::string& streamName, int truncate);
