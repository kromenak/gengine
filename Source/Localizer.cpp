#include "Localizer.h"

#include "IniParser.h"
#include "Services.h"
#include "StringUtil.h"
#include "TextAsset.h"

TYPE_DEF_BASE(Localizer);

Localizer::Localizer()
{
    TextAsset* textFile = Services::GetAssets()->LoadText("ESTRINGS.TXT");
    
    // Parse as INI file.
    IniParser parser(textFile->GetText(), textFile->GetTextLength());
    parser.SetMultipleKeyValuePairsPerLine(false);
    
    // Read one section at a time.
    // L10N files can have multiple sections, but we don't really care - just read it all.
    IniSection section;
    while(parser.ReadNextSection(section))
    {
        for(auto& line : section.lines)
        {
            IniKeyValue& entry = line.entries.front();
            mKeyToText[StringUtil::ToLowerCopy(entry.key)] = entry.value;
        }
    }
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
