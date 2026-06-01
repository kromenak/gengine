//
// Clark Kromenaker
//
// A Report Stream takes a log/report (as text) and performs a few actions:
//
// 1) If the stream is disabled, nothing happens.
//
// 2) Formats the text with additional header/footer decorators as configured.
//    Examples are adding the date/time, the name of the user or machine, separators, etc.
//
// 3) Outputs the formatted text to configuredlocations - console, file, etc.
//
// 4) Optionally, can trigger other actions (throw an exception).
//
#pragma once
#include <iostream>
#include <string>

#include "EnumClassFlags.h"
#include "StreamManager.h"

// Action to take when a report is logged.
enum class ReportAction
{
    Log,        // Log to outputs only.
    Fatal,      // Throw an exception and die.
    Prompt,     // Prompt user to continue.
};

// Output locations for a report.
enum class ReportOutput : int
{
    None            = 0,
    Console         = 1,    // Outputs contents to the console window.
    Debugger        = 2,    // Outputs to any active and supported debugger.
    File            = 4,    // Outputs to a file.
    SharedMemory    = 8,    // Outputs to shared memory region for external app debugging.
    OSDialog        = 16    // Original game called this "Win32Dialog"
};
ENUM_CLASS_FLAGS(ReportOutput);

// Content to include with a report.
enum class ReportContent : int
{
    None            = 0,
    Begin           = 1,    // Displays a header before the meat of the report.
                            // By default, just a bunch of dashes. Enable Category/Date/Time/etc to add to it.
    Content         = 2,    // The meat of the report. You probably want this enabled!
    End             = 4,    // Displays a line break after the content if enabled.

    // BELOW HERE: "Begin" content options
    Category        = 8,    // Displays report stream name in Begin header.
    Date            = 16,   // Displays date in Begin header.
    Time            = 32,   // Displays time in Begin header.
    //Debug           = 64,   // Doesn't seem to work in release or debug builds.
    Timeblock       = 128,  // Displays timeblock in Begin header.
    Location        = 256,  // Displays in-game location in Begin header.
    Machine         = 512,  // Displays machine name in Begin header.
    User            = 1024, // Displays the logged in user's name in Begin header.

    // BELOW HERE: Combinations of above flags.
    All             = (Begin | Content | End | Category | Date | Time | User | Machine | Timeblock | Location),
    AllButDateMachineUser = (Begin | Content | End | Category | Time | Timeblock | Location)
};
ENUM_CLASS_FLAGS(ReportContent);

class ReportStream
{
public:
    ReportStream(const std::string& name);
    ~ReportStream();

    void Log(const std::string& content);
    void Logf(const char* format, ...);
    void Logf(const char* format, va_list args);

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

    void SetFilePath(const std::string& filename);
    void SetFileTruncate(bool truncate) { mFileTruncate = truncate; }

private:
    // The name of the stream.
    std::string mName;

    // What action to perform when input is given to this stream.
    ReportAction mAction = ReportAction::Log;

    // Zero or more outputs to send any inputs to.
    ReportOutput mOutput = ReportOutput::None;

    // Zero or more contents to use when outputting.
    ReportContent mContent = ReportContent::None;

    // Logs will be written to a file at this path (if File is an output).
    std::string mFilePath;

    // The file stream to write to, if any.
    StreamHandle mFileStreamHandle = nullptr;

    // While each report stream can be configured to log to separate files, we also sometimes want to output to a shared log file.
    // The goal of this shared log is to mimic what is sent to the in-game console and/or standard output.
    StreamHandle mSharedLogFileStreamHandle = nullptr;

    // When writing to file, should we truncate (delete) any previous file contents?
    // If not, we just append new stuff onto the end of the existing file.
    bool mFileTruncate = false;

    // If not enabled, we ignore any inputs, don't trigger actions, don't send to outputs.
    bool mEnabled = true;

    std::string BuildOutputString(const std::string& content) const;
};
