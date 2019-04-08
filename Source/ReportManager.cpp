//
// ReportManager.cpp
//
// Clark Kromenaker
//
#include "ReportManager.h"

ReportManager::ReportManager()
{
	// Create a general purpose "generic" stream.
	ReportStream& generic = GetOrCreateStream("Generic");
	generic.SetAction(ReportAction::Log);
	generic.AddOutput(ReportOutput::Debugger);
	generic.AddOutput(ReportOutput::SharedMemory);
	generic.AddOutput(ReportOutput::Console);
	generic.AddContent(ReportContent::Content);
	
	// Create stream for GEngine.
	ReportStream& gengine = GetOrCreateStream("GEngine");
	gengine.SetAction(ReportAction::Log);
	gengine.AddOutput(ReportOutput::Debugger);
	gengine.AddOutput(ReportOutput::SharedMemory);
	gengine.AddOutput(ReportOutput::Console);
	gengine.AddContent(ReportContent::Content);
	
	// Create stream that outputs to OS dialog box.
	ReportStream& messageBox = GetOrCreateStream("MessageBox");
	messageBox.SetAction(ReportAction::Warning);
	messageBox.AddOutput(ReportOutput::File);
	messageBox.AddOutput(ReportOutput::OSDialog);
	messageBox.AddOutput(ReportOutput::Debugger);
	messageBox.AddContent(ReportContent::All);
	messageBox.SetFilename("errors.log");
	
	// Create stream that outputs to debugger console.
	ReportStream& debugger = GetOrCreateStream("Debugger");
	debugger.SetAction(ReportAction::Log);
	debugger.AddOutput(ReportOutput::Debugger);
	debugger.AddContent(ReportContent::Content);
	
	// Create stream for outputting status data.
	ReportStream& status = GetOrCreateStream("Status");
	status.SetAction(ReportAction::Log);
	status.AddOutput(ReportOutput::Debugger);
	status.AddOutput(ReportOutput::SharedMemory);
	status.AddOutput(ReportOutput::Console);
	status.AddContent(ReportContent::Content);
	
	// Create stream for outputting dumps.
	ReportStream& dump = GetOrCreateStream("Dump");
	dump.SetAction(ReportAction::Log);
	dump.AddOutput(ReportOutput::Debugger);
	dump.AddOutput(ReportOutput::SharedMemory);
	dump.AddOutput(ReportOutput::Console);
	dump.AddContent(ReportContent::Begin);
	dump.AddContent(ReportContent::Content);
	dump.AddContent(ReportContent::End);
	dump.AddContent(ReportContent::Category);
	dump.AddContent(ReportContent::Time);
	dump.AddContent(ReportContent::Timeblock);
	dump.AddContent(ReportContent::Location);
	
	// Create stream for warnings.
	ReportStream& warning = GetOrCreateStream("Warning");
	warning.SetAction(ReportAction::Warning);
	warning.AddOutput(ReportOutput::File);
	warning.AddOutput(ReportOutput::Debugger);
	warning.AddOutput(ReportOutput::SharedMemory);
	warning.AddOutput(ReportOutput::Console);
	warning.AddContent(ReportContent::All);
	warning.SetFilename("errors.log");
	
	// Create stream for errors.
	ReportStream& error = GetOrCreateStream("Error");
	error.SetAction(ReportAction::Error);
	error.AddOutput(ReportOutput::File);
	error.AddOutput(ReportOutput::Debugger);
	error.AddOutput(ReportOutput::SharedMemory);
	error.AddOutput(ReportOutput::Console);
	error.AddContent(ReportContent::All);
	error.SetFilename("errors.log");
	
	// Create stream for serious errors.
	ReportStream& seriousError = GetOrCreateStream("SeriousError");
	seriousError.SetAction(ReportAction::Error);
	seriousError.AddOutput(ReportOutput::File);
	seriousError.AddOutput(ReportOutput::Debugger);
	seriousError.AddOutput(ReportOutput::SharedMemory);
	seriousError.AddOutput(ReportOutput::Console);
	seriousError.AddContent(ReportContent::All);
	seriousError.SetFilename("errors.log");
	
	// Create stream for fatal errors.
	ReportStream& fatal = GetOrCreateStream("Fatal");
	fatal.SetAction(ReportAction::Fatal);
	fatal.AddOutput(ReportOutput::File);
	fatal.AddOutput(ReportOutput::Debugger);
	fatal.AddContent(ReportContent::All);
	fatal.SetFilename("errors.log");
}

void ReportManager::EnableStream(const std::string& streamName)
{
	ReportStream& stream = GetOrCreateStream(streamName);
	stream.Enable();
}

void ReportManager::DisableStream(const std::string& streamName)
{
	ReportStream& stream = GetOrCreateStream(streamName);
	stream.Disable();
}

void ReportManager::AddStreamOutput(const std::string& streamName, ReportOutput output)
{
	ReportStream& stream = GetOrCreateStream(streamName);
	stream.AddOutput(output);
}

void ReportManager::RemoveStreamOutput(const std::string& streamName, ReportOutput output)
{
	ReportStream& stream = GetOrCreateStream(streamName);
	stream.RemoveOutput(output);
}

void ReportManager::ClearStreamOutput(const std::string& streamName)
{
	ReportStream& stream = GetOrCreateStream(streamName);
	stream.ClearOutput();
}

void ReportManager::AddStreamContent(const std::string& streamName, ReportContent content)
{
	ReportStream& stream = GetOrCreateStream(streamName);
	stream.AddContent(content);
}

void ReportManager::RemoveStreamContent(const std::string& streamName, ReportContent content)
{
	ReportStream& stream = GetOrCreateStream(streamName);
	stream.RemoveContent(content);
}

void ReportManager::ClearStreamContent(const std::string& streamName)
{
	ReportStream& stream = GetOrCreateStream(streamName);
	stream.ClearContent();
}

void ReportManager::SetStreamAction(const std::string& streamName, ReportAction action)
{
	ReportStream& stream = GetOrCreateStream(streamName);
	stream.SetAction(action);
}

void ReportManager::SetStreamFilename(const std::string& streamName, const std::string& filename)
{
	ReportStream& stream = GetOrCreateStream(streamName);
	stream.SetFilename(filename);
}

void ReportManager::SetStreamFileTruncate(const std::string& streamName, bool truncate)
{
	ReportStream& stream = GetOrCreateStream(streamName);
	stream.SetFileTruncate(truncate);
}

void ReportManager::Log(const std::string& streamName, const std::string& content)
{
	ReportStream& stream = GetOrCreateStream(streamName);
	stream.Log(content);
}

ReportStream& ReportManager::GetReportStream(const std::string& streamName)
{
	return GetOrCreateStream(streamName);
}

ReportStream& ReportManager::GetOrCreateStream(const std::string& streamName)
{
	// If we can find the stream in our map, return it.
	auto it = mStreams.find(streamName);
	if(it != mStreams.end())
	{
		return it->second;
	}
	
	// Otherwise, we need to create a stream with this name.
	// Emplace adds a new ReportStream using ReportStream(name) constructor.
	// Returns std::pair with iterator to added item/
	auto insertedIt = mStreams.emplace(streamName, streamName).first;
	return insertedIt->second;
}
