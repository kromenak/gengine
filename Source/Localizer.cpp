//
// Localizer.cpp
//
// Clark Kromenaker
//
#include "Localizer.h"

#include "IniParser.h"
#include "Services.h"
#include "StringUtil.h"

TYPE_DEF_BASE(Localizer);

Localizer::Localizer()
{
    unsigned int bufferSize = 0;
    char* buffer = Services::GetAssets()->LoadRaw("ESTRINGS.TXT", bufferSize);
    
    // Parse as INI file.
    IniParser parser(buffer, bufferSize);
    parser.SetMultipleKeyValuePairsPerLine(false);
    
    // Read one section at a time.
    // L10N files can havd multiple sections, but we don't really care - just read it all.
    IniSection section;
    while(parser.ReadNextSection(section))
    {
        for(auto& line : section.lines)
        {
            IniKeyValue& entry = line.entries.front();
            mKeyToText[StringUtil::ToLowerCopy(entry.key)] = entry.value;
        }
    }
    
    delete[] buffer;
}

std::string Localizer::GetText(const std::string& key) const
{
    // Return localized text, if we have it.
    auto it = mKeyToText.find(StringUtil::ToLowerCopy(key));
    if(it != mKeyToText.end())
    {
        return it->second;
    }
    
    // Return the key back if no localization exists.
    return key;
}
