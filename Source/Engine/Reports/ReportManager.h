//
// Clark Kromenaker
//
// "Reports" are the engine's logging mechanism. Reports are sent to one or more "Report Streams."
//
// Each Report Stream is configurable for output location, write actions, and formatting.
//
#pragma once
#include <string>

#include "Log.h"
#include "ReportStream.h"
#include "StringUtil.h"

class ReportManager
{
public:
    ReportManager();

    void EnableStream(const std::string& streamName);
    void DisableStream(const std::string& streamName);

    void AddStreamOutput(const std::string& streamName, ReportOutput output);
    void RemoveStreamOutput(const std::string& streamName, ReportOutput output);
    void ClearStreamOutput(const std::string& streamName);

    void AddStreamContent(const std::string& streamName, ReportContent content);
    void RemoveStreamContent(const std::string& streamName, ReportContent content);
    void ClearStreamContent(const std::string& streamName);

    void SetStreamAction(const std::string& streamName, ReportAction action);

    void SetStreamFilename(const std::string& streamName, const std::string& filename);
    void SetStreamFileTruncate(const std::string& streamName, bool truncate);

    void Log(const std::string& streamName, const std::string& content);
    void Logf(const std::string& streamName, const char* format, ...);

    ReportStream& GetReportStream(const std::string& streamName);

private:
    // All defined streams, keyed by stream name.
    std::string_map_ci<ReportStream> mStreams;
};

extern ReportManager gReportManager;

// Convenience macros for logging to common built-in log streams.
#define LOG_GENERIC(x, ...) gReportManager.Logf("Generic", x, __VA_ARGS__);
#define LOG_WARNING(x, ...) gReportManager.Logf("Warning", x, __VA_ARGS__);
#define LOG_ERROR(x, ...) gReportManager.Logf("Error", x, __VA_ARGS__);
#define LOG_FATAL(x, ...) gReportManager.Logf("Fatal", x, __VA_ARGS__);