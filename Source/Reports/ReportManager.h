//
// Clark Kromenaker
//
// A manager for reporting information to "report streams". Each report stream
// can output info to various locations (file, dialog, console), trigger an
// action due (normal logging, throw exception, etc), and consistently format
// output.
//
#pragma once
#include <string>

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
	
	// Subsystems may want to get a ReportStream and call functions on it directly.
	ReportStream& GetReportStream(const std::string& streamName);
	
	// ShowReportGraph
	// HideReportGraph

private:
	// All defined streams, keyed by stream name.
	std::unordered_map_ci<std::string, ReportStream> mStreams;
	
	ReportStream& GetOrCreateStream(const std::string& streamName);
};

extern ReportManager gReportManager;