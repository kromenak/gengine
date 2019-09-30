//
// Console.h
//
// Clark Kromenaker
//
// The console system in GK3.
// This is only the system - UI is a separate class.
//
#pragma once
#include <string>
#include <vector>

class Console
{
public:
	void AddOutput(std::string str);
	std::string GetOutput(int lineCount);
	std::string GetOutput(int index, int lineCount);
	
	void ExecuteCommand(std::string command);
	
	int GetCommandHistoryLength() const { return (int)mCommandHistory.size(); }
	std::string GetCommandFromHistory(int index);
	
private:
	// Max scrollback lines we will store.
	int mMaxScrollbackLength = 1000;
	
	// Max number of commands we will store in history.
	int mMaxCommandHistoryLength = 40;
	
	// History of executed commands.
	std::vector<std::string> mCommandHistory;
};
