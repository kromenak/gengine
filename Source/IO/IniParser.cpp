#include "IniParser.h"

#include <cstdlib>
#include <iostream>

#include "mstream.h"
#include "StringUtil.h"

float IniKeyValue::GetValueAsFloat() const
{
    return StringUtil::ToFloat(value);
}

int IniKeyValue::GetValueAsInt() const
{
    return StringUtil::ToInt(value);
}

bool IniKeyValue::GetValueAsBool() const
{
    return StringUtil::ToBool(value);
}

Vector2 IniKeyValue::GetValueAsVector2() const
{
    // We assume the string form of {4.23, 5.23}
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
    return Vector2(std::stof(firstNum), std::stof(secondNum));
}

Vector3 IniKeyValue::GetValueAsVector3() const
{
    // We generally assume the string form of {4.23, 5.23, 10.04}.
    // However, GK3 data has some occasional typos and inconsistencies we must account for.

    // Trim leading non-numeric values.
    // This is usually just '{', but other garbage data (from typos) exists in some cases.
    int firstNumIndex = -1;
    for(int i = 0; i < value.length(); ++i)
    {
        // Also need to check '-' for symbol preceding negative numbers.
        if(std::isdigit(value[i]) || value[i] == '-')
        {
            firstNumIndex = i;
            break;
        }
    }
    if(firstNumIndex == -1) { return Vector3::Zero; }

    // Trim training non-numeric values. Usually just '}'.
    std::string noBraces = value.substr(firstNumIndex);
    while(!noBraces.empty() && !std::isdigit(noBraces.back()))
    {
        noBraces.pop_back();
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
    std::string secondNum = noBraces.substr(firstCommaIndex + 1, secondCommaIndex - firstCommaIndex - 1);
    std::string thirdNum = noBraces.substr(secondCommaIndex + 1, std::string::npos);

    // Handle too many negative signs. Yes, this is actually an issue (e.g. Devil's Armchair).
    while(firstNum.size() >= 2 && firstNum[0] == '-' && firstNum[1] == '-')
    {
        firstNum.erase(0, 1);
    }
    while(secondNum.size() >= 2 && secondNum[0] == '-' && secondNum[1] == '-')
    {
        secondNum.erase(0, 1);
    }
    while(thirdNum.size() >= 2 && thirdNum[0] == '-' && thirdNum[1] == '-')
    {
        thirdNum.erase(0, 1);
    }
    
    // Convert to numbers and return.
    return Vector3(std::stof(firstNum), std::stof(secondNum), std::stof(thirdNum));
}

Color32 IniKeyValue::GetValueAsColor32() const
{
	// Assume string form of R/G/B
	std::size_t firstSlashIndex = value.find('/');
	if(firstSlashIndex == std::string::npos)
	{
		return Color32::Black;
	}
	std::size_t secondSlashIndex = value.find('/', firstSlashIndex + 1);
	if(secondSlashIndex == std::string::npos)
	{
		return Color32::Black;
	}
	
	// Split at slashes.
	std::string firstNum = value.substr(0, firstSlashIndex);
	std::string secondNum = value.substr(firstSlashIndex + 1, secondSlashIndex - firstSlashIndex - 1);
	std::string thirdNum = value.substr(secondSlashIndex + 1, std::string::npos);
	
	// Convert to number and return.
	return Color32(std::stoi(firstNum), std::stoi(secondNum), std::stoi(thirdNum));
}

Rect IniKeyValue::GetValueAsRect() const
{
	// We assume the string form of {4.23, 5.23, 10.04, 5.23}
	// Values are in order of x1, z1, x2, z2 for two points.
	// First, let's get rid of the braces.
    std::string noBraces = value;
    if(noBraces[0] == '{' && noBraces[noBraces.size() - 1] == '}')
    {
        noBraces = value.substr(1, value.length() - 2);
    }

    // Split into 4 elements, divided by commas.
    // In at least one instance, errant commas can trip this up, so discard empty elements.
    std::vector<std::string> elements = StringUtil::Split(noBraces, ',', true);
    if(elements.size() < 4)
    {
        return Rect();
    }

    // Convert to numbers and return.
	Vector2 p1(std::stof(elements[0]), std::stof(elements[1]));
	Vector2 p2(std::stof(elements[2]), std::stof(elements[3]));
    return Rect(p1, p2);
}

std::unordered_map<std::string, IniKeyValue> IniSection::GetAsMap() const
{
    std::unordered_map<std::string, IniKeyValue> map;
    for(auto& line : lines)
    {
        for(auto& entry : line.entries)
        {
            map[entry.key] = entry;
        }
    }
    return map;
}

IniParser::IniParser(const char* filePath)
{
    // Create stream to read from file.
    mStream = new std::ifstream(filePath, std::ios::in);
    if(!mStream->good())
    {
        std::cout << "IniParser can't read from file " << filePath << "!" << std::endl;
    }
}

IniParser::IniParser(const uint8_t* memory, uint32_t memoryLength)
{
    // Create stream to read from memory.
    mStream = new imstream(reinterpret_cast<const char*>(memory), memoryLength);
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

std::unordered_map<std::string, IniKeyValue> IniParser::ParseAllAsMap()
{
    // Parse all per usual.
    ParseAll();
    
    // Now generate map and return it.
    std::unordered_map<std::string, IniKeyValue> map;
    for(auto& section : mSections)
    {
        std::unordered_map<std::string, IniKeyValue> sectionMap = section.GetAsMap();
        map.insert(sectionMap.begin(), sectionMap.end());
    }
    return map;
}

std::vector<IniSection> IniParser::GetSections(const std::string& name)
{
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

IniSection IniParser::GetSection(const std::string& name)
{
    for(auto& section : mSections)
    {
        if(StringUtil::EqualsIgnoreCase(section.name, name))
        {
            return section;
        }
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
    sectionOut.lines.clear();
    
    // Always remember the current position *before* reading the next line, so we can set it back if needs be.
    imstream::pos_type currentPos = mStream->tellg();
    
    // Just read the whole file one line at a time...
	// GetLineSanitized ensures: no line break or tab characters, no whitespace before/after, removes comments, etc.
    std::string line;
    bool inBlockComment = false;
    while(StringUtil::GetLineSanitized(*mStream, line))
    {
        // Ignore empty lines.
        if(line.empty())
        {
            currentPos = mStream->tellg();
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
            // If there are already lines in the IniSection, this must be the header for the NEXT section.
            // So revert back to previous position and let caller know we are done reading the current section.
            if(sectionOut.lines.size() > 0)
            {
                mStream->seekg(currentPos);
                return true;
            }

            // Otherwise, this is the header for the current section - let's parse it.
            // Remove '[' and ']' from the header.
            std::size_t endHeaderIndex = line.find(']', 1);
            if(endHeaderIndex == std::string::npos)
            {
                // No ']' at end - still accept it, but don't need to remove char at end.
                sectionOut.name = line.substr(1, std::string::npos);
            }
            else
            {
                sectionOut.name = line.substr(1, endHeaderIndex - 1);
            }

            // If there's an equals sign, it means this section is conditional.
            // A conditional section may be ignored by game code if the condition is not met.
            // The condition is usually Sheepscript code.
            std::size_t equalsIndex = sectionOut.name.find('=');
            if(equalsIndex != std::string::npos)
            {
                sectionOut.condition = sectionOut.name.substr(equalsIndex + 1, std::string::npos);
                sectionOut.name = sectionOut.name.substr(0, equalsIndex);
            }

            currentPos = mStream->tellg();
            continue;
        }
        
        // From here: this is a normal line with key/value pair(s) on it.
		// Create a line entry for the section and grab a reference to it.
		sectionOut.lines.emplace_back();
		IniLine& iniLine = sectionOut.lines.back();
		
        // We need to split the line into individual key/value pairs.
        while(!line.empty())
        {
            // First, determine the token we want to work with on the current line.
            // We want the first item, if there are multiple comma-separated values on a single line.
            // Otherwise, we just want the whole remaining line.
            std::string currentKeyValuePair;
            
			// If a line can have multiple key/value pairs, we'll need to determine
			// what portion of the current line constitutes the next key/value pair.
            if(mMultipleKeyValuePairsPerLine)
            {
				// Need to find index of a comma, which is the delimiter between key/value pairs on a single line.
                // We can't just use string::find because we want to ignore commas that are inside braces.
                // Ex: pos={10, 20, 30} should NOT be considered multiple key/value pairs.
				std::size_t delimiterIndex = std::string::npos;
                int braceDepth = 0;
                for(int i = 0; i < line.length(); i++)
                {
                    if(line[i] == '{') { braceDepth++; }
                    if(line[i] == '}') { braceDepth--; }
                    
                    if(line[i] == ',' && braceDepth == 0)
                    {
                        delimiterIndex = i;
                        break;
                    }
                }
                
                // If we found a valid delimiter, we only want to deal with the parts in front of the delimiter.
                // If no delimiter, then the rest of the line is our focus.
                if(delimiterIndex != std::string::npos)
                {
                    currentKeyValuePair = line.substr(0, delimiterIndex);
                    line = line.substr(delimiterIndex + 1, std::string::npos);
                }
                else
                {
                    currentKeyValuePair = line;
                    line.clear();
                }
            }
            else
            {
				// If no multiple key/value pairs per line, this is easy: the whole line is one key/value pair!
                currentKeyValuePair = line;
                line.clear();
            }
            
			// Build the key/value object, to be filled with data next.
			iniLine.entries.emplace_back();
			IniKeyValue& keyValue = iniLine.entries.back();
			
			// The delimiter between keys and values is the '=' symbol (e.g. "model=gab")
			// If there is no delimiter (fairly common), it still works, but it's just a key with no value (e.g. "hidden").
            std::size_t equalsIndex = currentKeyValuePair.find('=');
            if(equalsIndex != std::string::npos)
            {
                keyValue.key = currentKeyValuePair.substr(0, equalsIndex);
                keyValue.value = currentKeyValuePair.substr(equalsIndex + 1, std::string::npos);
				
				// If the key/value line had any spaces around the equal sign, we also want to get rid of those after splitting.
				StringUtil::TrimWhitespace(keyValue.key);
				StringUtil::TrimWhitespace(keyValue.value);
            }
            else
            {
                // Trim any whitespace from the key/value pair.
                StringUtil::TrimWhitespace(currentKeyValuePair);

				// In this case, set "key" and "value" to same thing so that we can still use "value" and value getters.
				//TODO: Seems kind of wasteful - perhaps we can say "use key field only" in this case; may want to augment/change GetValueAsX functions to work with this.
                keyValue.key = currentKeyValuePair;
                keyValue.value = currentKeyValuePair;
            }
        }
        currentPos = mStream->tellg();
    }
    
    // If we run out of things to read, return true if there's any data.
	// Remember, this function returns true if "sectionOut" has been filled with new data!
    return (!sectionOut.name.empty() || !sectionOut.condition.empty() || !sectionOut.lines.empty());
}

bool IniParser::ReadLine()
{
    if(mStream->eof()) { return false; }
    
    std::string line;
    while(StringUtil::GetLineSanitized(*mStream, line))
    {
        // Ignore empty lines.
        if(line.empty()) { continue; }
        
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
    
    // Trim off any comment on the line.
	StringUtil::TrimComment(currentKeyValuePair);
	
	// Get rid of any rogue tab characters.
	StringUtil::RemoveAll(currentKeyValuePair, '\t');
	
	// Trim any whitespace.
	StringUtil::Trim(currentKeyValuePair);
    
    // OK, so now we have a string representing a key/value pair, "model=blahblah" or similar.
    // But it might also just be a keyword (no value) like "hidden".
    found = currentKeyValuePair.find('=');
    if(found != std::string::npos)
    {
        mCurrentKeyValue.key = currentKeyValuePair.substr(0, found);
        mCurrentKeyValue.value = currentKeyValuePair.substr(found + 1, std::string::npos);
		
		// Ooof, we may also have to trim these now...
		StringUtil::Trim(mCurrentKeyValue.key);
		StringUtil::Trim(mCurrentKeyValue.value);
    }
    else
    {
        mCurrentKeyValue.key = currentKeyValuePair;
    }
    return true;
}
