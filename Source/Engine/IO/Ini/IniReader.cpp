#include "IniReader.h"

#include <iostream>

#include "mstream.h"
#include "StringUtil.h"
#include "TextReader.h"

IniReader::IniReader(const char* filePath) :
    mTextReader(filePath)
{

}

IniReader::IniReader(const uint8_t* memory, uint32_t memoryLength) :
    mTextReader(reinterpret_cast<const char*>(memory), memoryLength)
{

}

IniReader::IniReader(std::istream* stream) :
    mTextReader(stream)
{

}

void IniReader::ReadAll()
{
    // Already read data in, can early out.
    if(!mSections.empty()) { return; }

    // Make sure we're at the beginning of the file.
    mTextReader.Seek(0);

    // Read in each section and add to sections list.
    IniSection section;
    while(ReadNextSection(section))
    {
        mSections.push_back(section);
    }
}

std::unordered_map<std::string, IniKeyValue> IniReader::ReadAllAsMap()
{
    // Make sure all sections are read into memory.
    ReadAll();

    // Now generate map and return it.
    std::unordered_map<std::string, IniKeyValue> map;
    for(auto& section : mSections)
    {
        std::unordered_map<std::string, IniKeyValue> sectionMap = section.GetAsMap();
        map.insert(sectionMap.begin(), sectionMap.end());
    }
    return map;
}

std::vector<IniSection> IniReader::GetSections(const std::string& name)
{
    // Find all sections with the specified name and return them.
    // This allows for situations where multiple sections have the same name.
    std::vector<IniSection> toReturn;
    for(auto& section : mSections)
    {
        if(StringUtil::EqualsIgnoreCase(section.name, name))
        {
            toReturn.push_back(section);
        }
    }
    return toReturn;
}

IniSection IniReader::GetSection(const std::string& name)
{
    // Find the first section with the specified name and return it.
    for(auto& section : mSections)
    {
        if(StringUtil::EqualsIgnoreCase(section.name, name))
        {
            return section;
        }
    }
    return IniSection();
}

bool IniReader::ReadNextSection(IniSection& sectionOut)
{
    // Clear passed in section to default state.
    sectionOut.name.clear();
    sectionOut.condition.clear();
    sectionOut.lines.clear();

    // Clear last read section name and condition - these should be populated by reading the upcoming lines.
    mCurrentSectionName.clear();
    mCurrentSectionCondition.clear();

    // Read until we hit the next line containing key/value data.
    // If this section has no lines yet, don't stop at a section header - this will be *this* section's header.
    // If this section already has lines, stop at a section header - that is for the *next* section.
    while(ReadNextKeyValueLine(!sectionOut.lines.empty()))
    {
        sectionOut.lines.emplace_back();
        IniLine& iniLine = sectionOut.lines.back();

        // Read each key/value pair for this line.
        while(ParseKeyValuePairFromCurrentLine())
        {
            iniLine.entries.push_back(mCurrentKeyValue);
        }
    }

    // Save section name and condition.
    sectionOut.name = mCurrentSectionName;
    sectionOut.condition = mCurrentSectionCondition;

    // This function returns true if "sectionOut" has been filled with any data!
    return (!sectionOut.name.empty() || !sectionOut.condition.empty() || !sectionOut.lines.empty());
}

bool IniReader::ReadLine()
{
    // Just read the next value key/value line, skipping any headers encountered.
    return ReadNextKeyValueLine(false);
}

bool IniReader::ReadKeyValuePair()
{
    return ParseKeyValuePairFromCurrentLine();
}

bool IniReader::ReadNextKeyValueLine(bool stopAtSectionHeader)
{
    // Remember the current position *before* reading the next line, so we can set it back if needs be.
    imstream::pos_type currentPos = mTextReader.GetPosition();

    // Keep reading the next line until we want to stop.
    // We always want to skip comments and empty lines, and sometimes we want to skip section headers.
    std::string line;
    bool inBlockComment = false;
    while(mTextReader.ReadLine(line))
    {
        // SanitizeLine ensures: no line break or tab characters, no whitespace before/after, removes comments, etc.
        StringUtil::SanitizeLine(line);

        // Ignore empty lines.
        if(line.empty())
        {
            currentPos = mTextReader.GetPosition();
            continue;
        }

        // Handle/ignore block comments. Starts with "/*" and ends with "*/".
        // NOTE: this logic is not perfect (what if block comment starts at end of a valid line? or a valid line exists after an end?).
        // NOTE: but this might be fine for current needs.
        if(StringUtil::StartsWith(line, "/*"))
        {
            inBlockComment = true;
            continue;
        }
        if(inBlockComment)
        {
            if(StringUtil::Contains(line, "*/"))
            {
                inBlockComment = false;
            }
            continue;
        }

        // Check if this is an INI header (e.g. [General], [Hacks]).
        // Only check the '[' b/c GK3 sometimes forgets to put a ']' :P
        if(line.length() > 2 && line[0] == '[')
        {
            // If we want to stop reading at the next section header, revert reader to position before reading this line and return.
            if(stopAtSectionHeader)
            {
                mTextReader.Seek(currentPos);
                return false;
            }

            // Otherwise, this is the header for the current section - let's parse it.
            // Remove '[' and ']' from the header.
            std::size_t endHeaderIndex = line.find(']', 1);
            if(endHeaderIndex == std::string::npos)
            {
                // No ']' at end - still accept it, but don't need to remove char at end.
                mCurrentSectionName = line.substr(1, std::string::npos);
            }
            else
            {
                mCurrentSectionName = line.substr(1, endHeaderIndex - 1);
            }

            // If there's an equals sign, it means this section is conditional.
            // A conditional section may be ignored by game code if the condition is not met.
            // The condition is usually Sheepscript code.
            std::size_t equalsIndex = mCurrentSectionName.find('=');
            if(equalsIndex != std::string::npos)
            {
                mCurrentSectionCondition = mCurrentSectionName.substr(equalsIndex + 1, std::string::npos);
                mCurrentSectionName = mCurrentSectionName.substr(0, equalsIndex);
            }

            currentPos = mTextReader.GetPosition();
            continue;
        }

        // We have a new line containing key/value pair(s).
        mCurrentLine = line;
        mCurrentLineWorking = line;
        return true;
    }

    // We couldn't find another line containing key/value pair(s).
    return false;
}

bool IniReader::ParseKeyValuePairFromCurrentLine()
{
    if(mCurrentLineWorking.empty())
    {
        return false;
    }

    // First, determine the token we want to work with on the current line.
    // We want the first item, if there are multiple comma-separated values.
    // Otherwise, we just want the whole remaining line.
    std::string currentKeyValuePair;

    // If a line can have multiple key/value pairs, we'll need to determine
    // what portion of the current line constitutes the next key/value pair.
    std::size_t delimiterIndex = std::string::npos;
    if(mMultipleKeyValuePairsPerLine)
    {
        // Need to find index of a comma, which is the delimiter between key/value pairs on a single line.
        // We can't just use string::find because we want to ignore commas that are inside braces.
        // Ex: pos={10, 20, 30} should NOT be considered multiple key/value pairs.
        int braceDepth = 0;
        for(int i = 0; i < mCurrentLineWorking.length(); i++)
        {
            if(mCurrentLineWorking[i] == '{') { braceDepth++; }
            if(mCurrentLineWorking[i] == '}') { braceDepth--; }

            if(mCurrentLineWorking[i] == ',' && braceDepth == 0)
            {
                delimiterIndex = i;
                break;
            }
        }

        // If we found a valid comma separator, then we only want to deal with the parts in front of the comma.
        // If no comma, then the rest of the line is our focus.
        if(delimiterIndex != std::string::npos)
        {
            currentKeyValuePair = mCurrentLineWorking.substr(0, delimiterIndex);
            mCurrentLineWorking = mCurrentLineWorking.substr(delimiterIndex + 1, std::string::npos);
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

    // Trim off any comment on the line.
    StringUtil::TrimComment(currentKeyValuePair);

    // Get rid of any rogue tab characters.
    StringUtil::RemoveAll(currentKeyValuePair, '\t');

    // Trim any whitespace.
    StringUtil::TrimWhitespace(currentKeyValuePair);

    // OK, so now we have a string representing a key/value pair, "model=blahblah" or similar.
    // But it might also just be a keyword (no value) like "hidden".
    delimiterIndex = currentKeyValuePair.find('=');
    if(delimiterIndex != std::string::npos)
    {
        mCurrentKeyValue.key = currentKeyValuePair.substr(0, delimiterIndex);
        mCurrentKeyValue.value = currentKeyValuePair.substr(delimiterIndex + 1, std::string::npos);

        // Ooof, we may also have to trim these now...
        StringUtil::Trim(mCurrentKeyValue.key);
        StringUtil::Trim(mCurrentKeyValue.value);
    }
    else
    {
        mCurrentKeyValue.key = currentKeyValuePair;
        mCurrentKeyValue.value = currentKeyValuePair;
    }
    return true;
}
