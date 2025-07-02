//
// Clark Kromenaker
//
// A stream takes as input logs/info/reports (as text). It optionally outputs
// that input to some configurable locations (file, console, etc). It also
// optionally triggers some action due to receiving input (simply log, throw exception, etc).
//
#pragma once
#include <string>

#include "EnumClassFlags.h"

enum class ReportAction
{
    Log,
    Warning,
    Error,
    Fatal,
    Prompt,
    ResTrack,
};

enum class ReportOutput : int
{
    None            = 0,
    Console 		= 1,	// Outputs contents to the console window.
    Debugger 		= 2,	// Outputs to any active and supported debugger.
    File 			= 4,	// Outputs to a file.
    SharedMemory	= 8,	// Outputs to shared memory region for external app debugging.
    OSDialog 		= 16	// Original game called this "Win32Dialog"
};
ENUM_CLASS_FLAGS(ReportOutput);

enum class ReportContent : int
{
    None			= 0,
    Begin			= 1,	// Displays a header before the meat of the report.
                            // By default, just a bunch of dashes. Enable Category/Date/Time/etc to add to it.
    Content 		= 2,	// The meat of the report. You probably want this enabled!
    End 			= 4, 	// Displays a line break after the content if enabled.

    // BELOW HERE: "Begin" content options
    Category 		= 8, 	// Displays report stream name in Begin header.
    Date 			= 16,	// Displays date in Begin header.
    Time 			= 32,	// Displays time in Begin header.
    Debug 			= 64,	// Doesn't seem to work in release or debug builds.
    Timeblock 		= 128,	// Displays timeblock in Begin header.
    Location		= 256,	// Displays in-game location in Begin header.
    Machine			= 512,	// Displays machine name in Begin header.
    User			= 1024,	// Displays the logged in user's name in Begin header.

    // BELOW HERE: Combinations of above flags.
    All 			= (Begin | Content | End | Category | Date | Time | User | Machine | Timeblock | Location)
};
ENUM_CLASS_FLAGS(ReportContent);

class ReportStream
{
public:
    ReportStream() = default;
    ReportStream(const std::string& name);

    void Log(const std::string& content);

    const std::string& GetName() { return mName; }

    void SetAction(ReportAction action) { mAction = action; }

    void AddOutput(ReportOutput output) { mOutput = mOutput | output; }
    void RemoveOutput(ReportOutput output) { mOutput = mOutput & ~(output); }
    void ClearOutput() { mOutput = ReportOutput::None; }

    void AddContent(ReportContent content) { mContent = mContent | content; }
    void RemoveContent(ReportContent content) { mContent = mContent & ~(content); }
    void ClearContent() { mContent = ReportContent::None; }

    void Enable() { mEnabled = true; }
    void Disable() { mEnabled = false; }

    void SetFilename(const std::string& filename) { mFilename = filename; }
    void SetFileTruncate(bool truncate) { mFileTruncate = truncate; }

private:
    // The name of the stream.
    std::string mName;

    // What action to perform when input is given to this stream.
    ReportAction mAction = ReportAction::Log;

    // Zero or more outputs to send any inputs to.
    ReportOutput mOutput = (ReportOutput)0;

    // Zero or more contents to use when outputting.
    ReportContent mContent = (ReportContent)0;

    // When writing to file, the name of the file to write to.
    std::string mFilename;

    // When writing to file, should we truncate (delete) any previous file contents?
    // If not, we just append new stuff onto the end of the existing file.
    bool mFileTruncate = false;

    // If not enabled, we ignore any inputs, don't trigger actions, don't send to outputs.
    bool mEnabled = true;

    // Some variables present in original engine, but I don't know what to use them for yet.
    //std::string mBuffer;
    //int mBlockLevel = 0;
    //bool mReporting = false;

    std::string BuildOutputString(const std::string& content);
};
