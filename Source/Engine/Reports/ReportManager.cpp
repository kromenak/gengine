#include "ReportManager.h"

ReportManager gReportManager;

ReportManager::ReportManager()
{
    // The "generic" stream is a catchall if there isn't a more specific stream.
    ReportStream& generic = GetReportStream("Generic");
    generic.AddOutput(ReportOutput::Debugger | ReportOutput::SharedMemory | ReportOutput::Console);
    generic.AddContent(ReportContent::Content);

    // A stream for warnings.
    ReportStream& warning = GetReportStream("Warning");
    warning.AddOutput(ReportOutput::File |
                      ReportOutput::Debugger |
                      ReportOutput::SharedMemory |
                      ReportOutput::Console);
    warning.AddContent(ReportContent::All);
    warning.SetFilePath("Errors.log");

    // A stream for errors.
    ReportStream& error = GetReportStream("Error");
    error.AddOutput(ReportOutput::File |
                    ReportOutput::Debugger |
                    ReportOutput::SharedMemory |
                    ReportOutput::Console);
    error.AddContent(ReportContent::All);
    error.SetFilePath("Errors.log");

    // A stream for *serious* errors.
    ReportStream& seriousError = GetReportStream("SeriousError");
    seriousError.AddOutput(ReportOutput::File |
                           ReportOutput::Debugger |
                           ReportOutput::SharedMemory |
                           ReportOutput::Console);
    seriousError.AddContent(ReportContent::All);
    seriousError.SetFilePath("Errors.log");

    // A stream for **fatal** errors.
    // Writing to this stream basically will quit the game with an error popup.
    ReportStream& fatal = GetReportStream("Fatal");
    fatal.SetAction(ReportAction::Fatal);
    fatal.AddOutput(ReportOutput::File | ReportOutput::Debugger);
    fatal.AddContent(ReportContent::All);
    fatal.SetFilePath("Errors.log");

    // An error stream that will display a message box to the user.
    ReportStream& messageBox = GetReportStream("MessageBox");
    messageBox.AddOutput(ReportOutput::File | ReportOutput::OSDialog | ReportOutput::Debugger);
    messageBox.AddContent(ReportContent::All);
    messageBox.SetFilePath("Errors.log");

    // A general purpose stream that outputs to the in-game console.
    ReportStream& console = GetReportStream("Console");
    console.AddOutput(ReportOutput::Debugger | ReportOutput::SharedMemory | ReportOutput::Console);
    console.AddContent(ReportContent::AllButDateMachineUser);

    // A general purpose stream that outputs to the debugger.
    ReportStream& debugger = GetReportStream("Debugger");
    debugger.AddOutput(ReportOutput::Debugger);
    debugger.AddContent(ReportContent::Content);

    // A stream for dumping data. Used by all "DumpX" commands.
    ReportStream& dump = GetReportStream("Dump");
    dump.AddOutput(ReportOutput::Debugger |
                   ReportOutput::SharedMemory |
                   ReportOutput::Console);
    dump.AddContent(ReportContent::AllButDateMachineUser);
}

void ReportManager::EnableStream(const std::string& streamName)
{
    ReportStream& stream = GetReportStream(streamName);
    stream.Enable();
}

void ReportManager::DisableStream(const std::string& streamName)
{
    ReportStream& stream = GetReportStream(streamName);
    stream.Disable();
}

void ReportManager::AddStreamOutput(const std::string& streamName, ReportOutput output)
{
    ReportStream& stream = GetReportStream(streamName);
    stream.AddOutput(output);
}

void ReportManager::RemoveStreamOutput(const std::string& streamName, ReportOutput output)
{
    ReportStream& stream = GetReportStream(streamName);
    stream.RemoveOutput(output);
}

void ReportManager::ClearStreamOutput(const std::string& streamName)
{
    ReportStream& stream = GetReportStream(streamName);
    stream.ClearOutput();
}

void ReportManager::AddStreamContent(const std::string& streamName, ReportContent content)
{
    ReportStream& stream = GetReportStream(streamName);
    stream.AddContent(content);
}

void ReportManager::RemoveStreamContent(const std::string& streamName, ReportContent content)
{
    ReportStream& stream = GetReportStream(streamName);
    stream.RemoveContent(content);
}

void ReportManager::ClearStreamContent(const std::string& streamName)
{
    ReportStream& stream = GetReportStream(streamName);
    stream.ClearContent();
}

void ReportManager::SetStreamAction(const std::string& streamName, ReportAction action)
{
    ReportStream& stream = GetReportStream(streamName);
    stream.SetAction(action);
}

void ReportManager::SetStreamFilename(const std::string& streamName, const std::string& filename)
{
    ReportStream& stream = GetReportStream(streamName);
    stream.SetFilePath(filename);
}

void ReportManager::SetStreamFileTruncate(const std::string& streamName, bool truncate)
{
    ReportStream& stream = GetReportStream(streamName);
    stream.SetFileTruncate(truncate);
}

void ReportManager::Log(const std::string& streamName, const std::string& content)
{
    GetReportStream(streamName).Log(content);
}

void ReportManager::Logf(const std::string& streamName, const char* format, ...)
{
    // Pass va_list to stream's Logf function.
    va_list args;
    va_start(args, format);
    GetReportStream(streamName).Logf(format, args);
    va_end(args);
}

ReportStream& ReportManager::GetReportStream(const std::string& streamName)
{
    // If we can find the stream in our map, return it.
    auto it = mStreams.find(streamName);
    if(it != mStreams.end())
    {
        return it->second;
    }

    // Otherwise, we need to create a stream with this name.
    // Emplace adds a new ReportStream using ReportStream(name) constructor.
    // Returns std::pair with iterator to added item.
    auto insertedIt = mStreams.emplace(streamName, streamName).first;
    return insertedIt->second;
}
