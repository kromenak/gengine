//
// IniParser.cpp
//
// Clark Kromenaker
//
#include "IniParser.h"
#include <iostream>
#include <cstdlib>
#include "imstream.h"
#include "StringUtil.h"

bool IniKeyValue::GetValueAsBool()
{
    if(StringUtil::EqualsIgnoreCase(value, "yes"))
    {
        return true;
    }
    else if(StringUtil::EqualsIgnoreCase(value, "no"))
    {
        return false;
    }
    else if(StringUtil::EqualsIgnoreCase(value, "true"))
    {
        return true;
    }
    else if(StringUtil::EqualsIgnoreCase(value, "false"))
    {
        return false;
    }
    
    // Unknown - return false?
    return false;
}

Vector2 IniKeyValue::GetValueAsVector2()
{
    // We assume the string form of {4.23, 5.23} as an example.
    // First, let's get rid of the braces.
    std::string noBraces = value;
    if(noBraces[0] == '{' && noBraces[noBraces.size() - 1] == '}')
    {
        noBraces = value.substr(1, value.length() - 2);
    }
    
    // Find the comma index. If not present,
    // this isn't the right form, so we fail.
    std::size_t commaIndex = noBraces.find(',');
    if(commaIndex == std::string::npos)
    {
        return Vector2::Zero;
    }
    
    // Split at the comma.
    std::string firstNum = noBraces.substr(0, commaIndex);
    std::string secondNum = noBraces.substr(commaIndex + 1, std::string::npos);
    
    // Convert to numbers and return.
    return Vector2(atof(firstNum.c_str()), atof(secondNum.c_str()));
}

Vector3 IniKeyValue::GetValueAsVector3()
{
    // We assume the string form of {4.23, 5.23, 10.04} as an example.
    // First, let's get rid of the braces.
    std::string noBraces = value;
    if(noBraces[0] == '{' && noBraces[noBraces.size() - 1] == '}')
    {
        noBraces = value.substr(1, value.length() - 2);
    }
    
    // Find the two commas.
    std::size_t firstCommaIndex = noBraces.find(',');
    if(firstCommaIndex == std::string::npos)
    {
        return Vector3::Zero;
    }
    std::size_t secondCommaIndex = noBraces.find(',', firstCommaIndex + 1);
    if(secondCommaIndex == std::string::npos)
    {
        return Vector3::Zero;
    }
    
    // Split at commas.
    std::string firstNum = noBraces.substr(0, firstCommaIndex);
    std::string secondNum = noBraces.substr(firstCommaIndex + 1, secondCommaIndex - firstCommaIndex);
    std::string thirdNum = noBraces.substr(secondCommaIndex + 1, std::string::npos);
    
    // Convert to numbers and return.
    return Vector3(atof(firstNum.c_str()), atof(secondNum.c_str()), atof(thirdNum.c_str()));
}

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

void IniParser::ParseAll()
{
    // Clear any existing sections.
    mSections.clear();
    
    // Reset cursor to top of file.
    ResetToTop();
    
    // Read in each section and add to list.
    IniSection section;
    while(ReadNextSection(section))
    {
        mSections.push_back(section);
    }
}

std::vector<IniSection> IniParser::GetSections(std::string name)
{
    std::vector<IniSection> toReturn;
    for(auto& section : mSections)
    {
        if(section.name == name)
        {
            toReturn.push_back(section);
        }
    }
    return toReturn;
}

IniSection IniParser::GetSection(std::string name)
{
    std::vector<IniSection> sections = GetSections(name);
    if(sections.size() > 0)
    {
        return GetSections(name)[0];
    }
    return IniSection();
}

void IniParser::ResetToTop()
{
    mStream->seekg(0);
}

bool IniParser::ReadNextSection(IniSection& sectionOut)
{
    // Make sure passed in section is empty.
    sectionOut.name.clear();
    sectionOut.condition.clear();
    sectionOut.entries.clear();
    
    // Always remember the current position *before* reading the next line,
    // so we can set it back if needs be.
    imstream::pos_type currentPos = mStream->tellg();
    
    // Just read the whole file one line at a time...
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
        
        // Trim the line of any whitespaces after getting rid of line breaks.
        StringUtil::Trim(line);
        StringUtil::Trim(line, '\t');
        
        // Ignore empty lines. Need to do this after \r check because some lines might just be '\r'.
        if(line.empty())
        {
            currentPos = mStream->tellg();
            continue;
        }
        
        // Ignore comment lines.
        if(line.length() >= 2 && line[0] == '/' && line[1] == '/')
        {
            currentPos = mStream->tellg();
            continue;
        }
        
        // Detect headers and react to them, but don't stop parsing.
        if(line.length() > 2 && line[0] == '[' && line[line.length() - 1] == ']')
        {
            if(sectionOut.entries.size() > 0)
            {
                mStream->seekg(currentPos);
                return true;
            }
            
            // Subtract the brackets to get the section name.
            sectionOut.name = line.substr(1, line.length() - 2);
            
            // If there's an equals sign, it means this section is conditional.
            std::size_t equalsIndex = sectionOut.name.find('=');
            if(equalsIndex != std::string::npos)
            {
                sectionOut.condition = sectionOut.name.substr(equalsIndex + 1, std::string::npos);
                sectionOut.name = sectionOut.name.substr(0, equalsIndex);
            }
            
            currentPos = mStream->tellg();
            continue;
        }
        
        // From here: just a normal line with key/value pair(s) on it.
        // So, we need to split it into individual key/value pairs.
        IniKeyValue* lastOnLine = nullptr;
        while(!line.empty())
        {
            // First, determine the token we want to work with on the current line.
            // We want the first item, if there are multiple comma-separated values.
            // Otherwise, we just want the whole remaining line.
            std::string currentKeyValuePair;
            
            std::size_t found = std::string::npos;
            if(mMultipleKeyValuePairsPerLine)
            {
                // We can't just use string::find because we want to ignore commas that are inside braces.
                // Ex: pos={10, 20, 30} should NOT be considered multiple key/value pairs.
                int braceDepth = 0;
                for(int i = 0; i < line.length(); i++)
                {
                    if(line[i] == '{') { braceDepth++; }
                    if(line[i] == '}') { braceDepth--; }
                    
                    if(line[i] == ',' && braceDepth == 0)
                    {
                        found = i;
                        break;
                    }
                }
                
                // If we found a valid comma separator, then we only want to deal with the parts in front of the comma.
                // If no comma, then the rest of the line is our focus.
                if(found != std::string::npos)
                {
                    currentKeyValuePair = line.substr(0, found);
                    line = line.substr(found + 1, std::string::npos);
                }
                else
                {
                    currentKeyValuePair = line;
                    line.clear();
                }
            }
            else
            {
                currentKeyValuePair = line;
                line.clear();
            }
            
            IniKeyValue* keyValue = new IniKeyValue();
            if(lastOnLine == nullptr)
            {
                sectionOut.entries.push_back(keyValue);
            }
            else
            {
                lastOnLine->next = keyValue;
            }
            lastOnLine = keyValue;
            
            // Trim any whitespace.
            StringUtil::Trim(currentKeyValuePair);
            
            // OK, so now we have a string representing a key/value pair, "model=blahblah" or similar.
            // But it might also just be a keyword (no value) like "hidden".
            found = currentKeyValuePair.find('=');
            if(found != std::string::npos)
            {
                keyValue->key = currentKeyValuePair.substr(0, found);
                keyValue->value = currentKeyValuePair.substr(found + 1, std::string::npos);
            }
            else
            {
                keyValue->key = currentKeyValuePair;
                keyValue->value = currentKeyValuePair;
            }
        }
        currentPos = mStream->tellg();
    }
    
    // If we run out of things to read, return true if there's any data.
    return (!sectionOut.name.empty() || !sectionOut.condition.empty() || !sectionOut.entries.empty());
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
    
    std::size_t found = std::string::npos;
    if(mMultipleKeyValuePairsPerLine)
    {
        // We can't just use string::find because we want to ignore commas that are inside braces.
        // Ex: pos={10, 20, 30} should not be considered multiple key/value pairs.
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
        mCurrentKeyValue.key = currentKeyValuePair.substr(0, found);
        mCurrentKeyValue.value = currentKeyValuePair.substr(found + 1, std::string::npos);
    }
    else
    {
        mCurrentKeyValue.key = currentKeyValuePair;
    }
    return true;
}
