#include "ReportStream.h"

#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>

#include "Console.h"
#include "GameProgress.h"
#include "GEngine.h"
#include "LocationManager.h"
#include "Log.h"
#include "OSDialog.h"
#include "Platform.h"
#include "StreamLockGuard.h"
#include "SystemUtil.h"

ReportStream::ReportStream(const std::string& name) :
    mName(name),
    mFilePath(name + ".log")
{

}

ReportStream::~ReportStream()
{
    // Return any stream handles we've obtained.
    StreamManager::ReturnStream(mFileStreamHandle);
}

void ReportStream::Log(const std::string& content)
{
    // Easy part: don't do anything if not enabled.
    if(!mEnabled) { return; }

    // Build output string based on desired contents.
    std::string output = BuildOutputString(content);

    // Handle console output type
    if((mOutput & ReportOutput::Console) != ReportOutput::None)
    {
        gConsole.AddToScrollback(output);
    }

    // Handle file output type.
    if((mOutput & ReportOutput::File) != ReportOutput::None)
    {
        // If no file stream yet, grab one.
        if(mFileStreamHandle == nullptr)
        {
            mFileStreamHandle = StreamManager::TakeFileStream(mFilePath, mFileTruncate);
        }

        // Lock the file stream, write, then unlock.
        StreamLockGuard lock(mFileStreamHandle);
        lock.stream << output;
    }

    // Handle shared memory output type (or rather...DON'T!).
    if((mOutput & ReportOutput::SharedMemory) != ReportOutput::None)
    {
        // Do nothing, not supported for now...
    }

    // Handle OS dialog output type
    if((mOutput & ReportOutput::OSDialog) != ReportOutput::None)
    {
        OSDialog::Ok(OSDIALOG_INFO, output);
    }

    // Handle debugger output type.
    if((mOutput & ReportOutput::Debugger) != ReportOutput::None)
    {
        // On Windows, we can output the string to the debugger (usually Visual Studio 20XX) "output" panel.
        #if defined(PLATFORM_WINDOWS)
        OutputDebugStringA(output.c_str());
        #endif

        // "::Log" automatically adds a newline to outputs, so if the output has a newline at the end, remove that.
        if(!output.empty() && output.back() == '\n')
        {
            output.pop_back();
        }

        // For most platforms, outputting to standard output via printf achieves the goal - the string appears in Xcode, CLion, etc.
        // Even on Windows, this is handy to see log output in the Command Prompt window when using /subsystem:console.
        ::Log(output.c_str());
    }

    // If this report stream has an action associated with it, take that action.
    if(mAction == ReportAction::Fatal)
    {
        OSDialog::Ok(OSDIALOG_ERROR,
                     "Cannot continue after previous error (category '" + mName + "') [see error log for more information]. Aborting...");
        GEngine::Instance()->Quit();
    }
    else if(mAction == ReportAction::Prompt)
    {
        if(!OSDialog::YesNo(OSDIALOG_ERROR,
                           "Continue Playing?",
                           "An error has occurred and we recommend that you quit and reload the game. Ignore this advice and keep playing anyway?"))
        {
            OSDialog::Ok(OSDIALOG_ERROR, "Smart move");
            GEngine::Instance()->Quit();
        }
    }
}

void ReportStream::Logf(const char* format, ...)
{
    // Convert to va_list and pass to other Logf.
    va_list args;
    va_start(args, format);
    Logf(format, args);
    va_end(args);
}

void ReportStream::Logf(const char* format, va_list args)
{
    // Format to an std::string and pass to the basic Log function.
    Log(StringUtil::vFormatf(format, args));
}

void ReportStream::SetFilePath(const std::string& filePath)
{
    // Has the file changed from what was previously set?
    if(filePath != mFilePath)
    {
        // Close the previous file stream, if any.
        if(mFileStreamHandle != nullptr)
        {
            StreamManager::ReturnStream(mFileStreamHandle);
            mFileStreamHandle = nullptr;
        }

        // Save the new file path. A stream will be opened the first time we write to this path.
        mFilePath = filePath;
    }
}

std::string ReportStream::BuildOutputString(const std::string& content) const
{
    // If we want "Begin" content, we'll add some data before the real content.
    std::ostringstream outputStr;
    bool addedContent = false;
    if((mContent & ReportContent::Begin) != ReportContent::None)
    {
        // The begin string always starts with 5 dashes.
        outputStr << "-----";

        // Go through each of the possible begin header bits that we could add to the begin content.
        // These are added in order with a * char between them. For example, we might get:
        // ----- 'Dump' * TB: '110a' * Loc: 'r25' * 03/16/2019 * 11:41:25 -----
        if((mContent & ReportContent::Category) != ReportContent::None)
        {
            outputStr << " '" << mName << "' ";
        }
        if((mContent & ReportContent::Machine) != ReportContent::None)
        {
            if(outputStr.tellp() > 5)
            {
                outputStr << "*";
            }
            outputStr << " " << SystemUtil::GetMachineName() << " ";
        }
        if((mContent & ReportContent::User) != ReportContent::None)
        {
            if(outputStr.tellp() > 5)
            {
                outputStr << "*";
            }
            outputStr << " " << SystemUtil::GetCurrentUserName() << " ";
        }
        if((mContent & ReportContent::Timeblock) != ReportContent::None)
        {
            if(outputStr.tellp() > 5)
            {
                outputStr << "*";
            }
            outputStr << " TB: '" << gGameProgress.GetTimeblock() << "' ";
        }
        if((mContent & ReportContent::Location) != ReportContent::None)
        {
            if(outputStr.tellp()> 5)
            {
                outputStr << "*";
            }
            outputStr << " Loc: '" << gLocationManager.GetLocation() << "' ";
        }
        if((mContent & ReportContent::Date) != ReportContent::None)
        {
            if(outputStr.tellp()> 5)
            {
                outputStr << "*";
            }

            // Outputs date in MM/dd/yyyy format.
            // Using setw/setfill to ensure leading zeros on single digit months/days.
            time_t now = time(0);
            tm* time = localtime(&now);
            outputStr << " " << std::setw(2) << std::setfill('0') << (time->tm_mon + 1) << "/";
            outputStr << std::setw(2) << std::setfill('0') << (time->tm_mday) << "/";
            outputStr << (time->tm_year + 1900) << " ";
        }
        if((mContent & ReportContent::Time) != ReportContent::None)
        {
            if(outputStr.tellp()> 5)
            {
                outputStr << "*";
            }

            // Outputs time in hh:mm:ss format.
            // Using setw/setfill to ensure leading zeros on single digit hours/minutes/seconds.
            time_t now = time(0);
            tm* time = localtime(&now);
            outputStr << " " << std::setw(2) << std::setfill('0') << time->tm_hour << ":";
            outputStr << std::setw(2) << std::setfill('0') << time->tm_min << ":";
            outputStr << std::setw(2) << std::setfill('0') << time->tm_sec << " ";
        }

        // If we added any begin content above, we simply cap off the begin string with 5 more dashes.
        // If NO begin content was added, the final string should be 25 dashes only.
        if(outputStr.tellp() > 5)
        {
            outputStr << "-----";
        }
        else
        {
            outputStr << "--------------------";
        }
        addedContent = true;
    }

    // If we want "Content" content, that means we want to output what was passed in!
    // It seems pretty rare to NOT do this...but you can!
    if((mContent & ReportContent::Content) != ReportContent::None)
    {
        // Add a newline before the main content if we added begin content.
        if(addedContent)
        {
            outputStr << "\n";
        }
        outputStr << content;
        addedContent = true;
    }

    // If we want "End" content, we'll add an empty line for spacing.
    if((mContent & ReportContent::End) != ReportContent::None)
    {
        if(addedContent)
        {
            outputStr << "\n";
        }
        outputStr << "\n";
    }
    return outputStr.str();
}
