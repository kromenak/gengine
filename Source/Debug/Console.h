//
// Clark Kromenaker
//
// The console system in GK3.
// This is only the system - UI is a separate class.
//
#pragma once
#include <string>
#include <vector>

#include "Queue.h"

class ReportStream;

class Console
{
public:
    void AddToScrollback(const std::string& str);
    const std::vector<std::string>& GetScrollback() const { return mScrollback; }

    void ExecuteCommand(const std::string& command);

    size_t GetCommandHistoryLength() const { return mCommandHistory.Size(); }
    const std::string& GetCommandFromHistory(size_t index) const;

    void SetReportStream(ReportStream* reportStream) { mConsoleReportStream = reportStream; }

private:
    // Max scrollback lines we will store.
    static const uint32_t kMaxScrollbackLength = 1000;

    // The scrollback buffer. Split into individual lines.
    // Newest items are at the end of the list.
    std::vector<std::string> mScrollback;

    // Max number of commands we will store in history.
    static const uint32_t kMaxCommandHistoryLength = 100;

    // History of executed commands.
    Queue<std::string, kMaxCommandHistoryLength> mCommandHistory;

    // Counts commands executed via console.
    // Used primarily as an identifier when compiling console commands.
    uint32_t mCommandCounter = 0;

    // Report stream for console output.
    ReportStream* mConsoleReportStream = nullptr;
};

extern Console gConsole;