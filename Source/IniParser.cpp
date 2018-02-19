//
// IniParser.cpp
//
// Clark Kromenaker
//
#include "IniParser.h"
#include <iostream>
#include "imstream.h"
#include "StringUtil.h"

IniParser::IniParser(const char* filePath)
{
    // Create stream to read from file.
    mStream = new std::ifstream(filePath, std::ios::in);
    if(!mStream->good())
    {
        std::cout << "IniParser can't read from file " << filePath << "!" << std::endl;
    }
    mCurrentSection = "";
}

IniParser::IniParser(const char* memory, unsigned int memoryLength)
{
    // Create stream to read from memory.
    mStream = new imstream(memory, memoryLength);
    mCurrentSection = "";
}

IniParser::~IniParser()
{
    delete mStream;
}

bool IniParser::ReadLine()
{
    if(mStream->eof()) { return false; }
    
    std::string line;
    while(std::getline(*mStream, line))
    {
        // "getline" reads up to the '\n' character in a file, and "eats" the '\n' too.
        // But Windows line breaks might include the '\r' character too, like "\r\n".
        // To deal with this semi-elegantly, we'll search for and remove the '\r' here.
        if(!line.empty() && line[line.length() - 1] == '\r')
        {
            line.resize(line.length() - 1);
        }
        
        // Ignore empty lines. Need to do this after \r check because some lines might just be '\r'.
        if(line.empty()) { continue; }
        
        // Ignore comment lines.
        if(line.length() > 2 && line[0] == '/' && line[1] == '/') { continue; }
        
        // Detect headers and react to them, but don't stop parsing.
        if(line.length() > 2 && line[0] == '[' && line[line.length() - 1] == ']')
        {
            mCurrentSection = line.substr(1, line.length() - 2);
            continue;
        }
        
        // Save the current line.
        mCurrentLine = line;
        mCurrentLineWorking = line;
        return true;
    }
    
    // If we get here, I guess it means we ran out of stuff to read.
    return false;
}

bool IniParser::ReadKeyValuePair()
{
    // If nothing left in current line, we're done.
    if(mCurrentLineWorking.empty()) { return false; }
    
    // First, determine the token we want to work with on the current line.
    // We want the first item, if there are multiple comma-separated values.
    // Otherwise, we just want the whole remaining line.
    std::string currentKeyValuePair;
    
    // We can't just use string::find because we want to ignore commans that are inside braces.
    // Ex: pos={10, 20, 30} should not be considered multiple key/value pairs.
    std::size_t found = std::string::npos;
    int braceDepth = 0;
    for(int i = 0; i < mCurrentLineWorking.length(); i++)
    {
        if(mCurrentLineWorking[i] == '{') { braceDepth++; }
        if(mCurrentLineWorking[i] == '}') { braceDepth--; }
        
        if(mCurrentLineWorking[i] == ',' && braceDepth == 0)
        {
            found = i;
            break;
        }
    }
    
    // If we found a valid comma separator, then we only want to deal with the parts in front of the comma.
    // If no comma, then the rest of the line is our focus.
    if(found != std::string::npos)
    {
        currentKeyValuePair = mCurrentLineWorking.substr(0, found);
        mCurrentLineWorking = mCurrentLineWorking.substr(found + 1, std::string::npos);
    }
    else
    {
        currentKeyValuePair = mCurrentLineWorking;
        mCurrentLineWorking.clear();
    }
    
    // Trim any whitespace.
    StringUtil::Trim(currentKeyValuePair);
    
    // OK, so now we have a string representing a key/value pair, "model=blahblah" or similar.
    // But it might also just be a keyword (no value) like "hidden".
    found = currentKeyValuePair.find('=');
    if(found != std::string::npos)
    {
        mCurrentKey = currentKeyValuePair.substr(0, found);
        mCurrentValue = currentKeyValuePair.substr(found + 1, std::string::npos);
    }
    else
    {
        mCurrentKey = currentKeyValuePair;
        mCurrentValue.clear();
    }
    return true;
}
