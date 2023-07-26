#include "ReportStream.h"

#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>

#include "Console.h"
#include "GameProgress.h"
#include "LocationManager.h"
#include "SystemUtil.h"

ReportStream::ReportStream(std::string name) :
	mName(name),
	mFilename(name + ".log")
{
	
}

void ReportStream::Log(std::string content)
{
	// Easy part: don't do anything if not enabled.
	if(!mEnabled) { return; }
	
	//TODO: Based on action, do something!
	
	// Build output string based on desired contents.
	std::string output = BuildOutputString(content);
	
	// Handle console output type
	if((mOutput & ReportOutput::Console) != ReportOutput::None)
	{
		gConsole.AddToScrollback(output);
	}
	
	// Handle debugger output type.
	if((mOutput & ReportOutput::Debugger) != ReportOutput::None)
	{
		std::cout << output;
	}
	
	//TODO: Handle file output type
	
	//TODO: Handle shared memory output type
	
	//TODO: Handle OS dialog output type
}

std::string ReportStream::BuildOutputString(const std::string& content)
{
	std::ostringstream outputStr;
	
	// If we want "Begin" content, we'll add some data before the real content.
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
		outputStr << std::endl;
	}
	
	// If we want "Content" content, that means we want to output what was passed in!
	// It seems pretty rare to NOT do this...but you can!
	if((mContent & ReportContent::Content) != ReportContent::None)
	{
		outputStr << content << std::endl;
	}
	
	// If we want "End" content, we'll add an empty line for spacing.
	if((mContent & ReportContent::End) != ReportContent::None)
	{
		outputStr << "\n";
	}
	return outputStr.str();
}
