#include "Console.h"

#include "LayerManager.h"
#include "ReportManager.h"
#include "SheepManager.h"
#include "SheepScript.h"
#include "StringUtil.h"

Console gConsole;

void Console::AddToScrollback(const std::string& str)
{
    // Push each line onto the scrollback.
	std::vector<std::string> lines = StringUtil::Split(str, '\n');
	for(auto& line : lines)
	{
		mScrollback.push_back(line);
	}

    // If scrollback is too large, erase old messages to reduce size.
	if(mScrollback.size() > kMaxScrollbackLength)
	{
        uint32_t extraCount = mScrollback.size() - kMaxScrollbackLength;
		mScrollback.erase(mScrollback.begin(), mScrollback.begin() + extraCount);
	}
}

void Console::ExecuteCommand(const std::string& command)
{
	// Passing an empty command outputs 40 dashes.
	// An acknowledgement that you did something meaningless.
	if(command.empty())
	{
		AddToScrollback("----------------------------------------");
		return;
	}

    // Any console command registered is itself output to the Console log stream...
	gReportManager.Log("Console", StringUtil::Format("console command: '%s'", command.c_str()));

	// Modify command to have required syntax.
	//TODO: Update compiler to accept without braces?
	std::string modCommand = command;
	if(modCommand[0] != '{')
	{
		modCommand.insert(modCommand.begin(), '{');
	}
	if(modCommand[modCommand.size() - 1] != '}')
	{
		modCommand.push_back('}');
	}

	// Compile the sheep from text.
	std::string scriptName = StringUtil::Format("`Console`:%i", mCommandCounter);
	SheepScript* sheepScript = gSheepManager.Compile(scriptName, modCommand);

	// If compiled successfully, execute it!
    // Execute this as a "Global" sheep (attached to the bottom layer) to avoid it being killed if it pops the top layer.
	if(sheepScript != nullptr)
	{
		gSheepManager.Execute(sheepScript, [sheepScript]() {
			delete sheepScript;
		}, gLayerManager.GetBottomLayerName());
	}

    // Add command to history.
    // Commands are usually added to history, unless it matches the most recent or second-most-recent commands.
    size_t historySize = mCommandHistory.size();
    if(historySize > 0 && StringUtil::EqualsIgnoreCase(mCommandHistory[historySize - 1], command))
    {
        // Do nothing - don't add this command to history, since the most recent history item exactly matches it.
    }
    else if(historySize > 1 && StringUtil::EqualsIgnoreCase(mCommandHistory[historySize - 2], command))
    {
        // When the second-to-last history command is executed, the behavior is kind of unexpected/interesting.
        // The second-to-last and last commands in history swap!
        std::swap(mCommandHistory[historySize - 1], mCommandHistory[historySize - 2]);
    }
    else
    {
        // Just add to the history in normal way.
        mCommandHistory.push_back(command);
    }

	// Increment command counter.
	mCommandCounter++;
}

const std::string& Console::GetCommandFromHistory(size_t index) const
{
	return mCommandHistory[index];
}
