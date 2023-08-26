#include "Localizer.h"

#include <cassert>

#include "AssetManager.h"
#include "IniParser.h"
#include "TextAsset.h"

Localizer gLocalizer;

Localizer::Localizer(const std::string& fileName, const std::string& sectionName)
{
    Load(fileName, sectionName);
}

void Localizer::Load(const std::string& fileName, const std::string& sectionName)
{
    // We assume the passed in filename does not have the language prefix attached yet.
    // For now, we'll assume English, but we could add other localizations pretty easily.
    std::string localeFileName = "E" + fileName;

    // GK3 translations are stored in INI files with various sections.
    // Sometimes, we don't care about the sections - we just load the entire file into a single localizer.
    // However, sometimes sections have duplicate keys - in that case, it's better to treat each section as its own localizer.
    TextAsset* textFile = gAssetManager.LoadText(localeFileName, AssetScope::Manual);

    // Parse as INI file.
    // Ignore multiple key values per line b/c a comma is going to be part of the translation copy, rather than another key/value on the same line.
    IniParser parser(textFile->GetText(), textFile->GetTextLength());
    parser.SetMultipleKeyValuePairsPerLine(false);

    // Read one section at a time.
    // L10N files can have multiple sections. Depending on whether a section name is passed in, we might only care about one section.
    IniSection section;
    while(parser.ReadNextSection(section))
    {
        // If section name was passed in, and this isn't the desired section, skip it!
        if(!sectionName.empty() && !StringUtil::EqualsIgnoreCase(section.name, sectionName))
        {
            continue;
        }

        // Each line represents a translation.
        for(auto& line : section.lines)
        {
            IniKeyValue& entry = line.entries.front();

            // GK3 has some inconsistent commenting styles. The vast majority of data files use "//".
            // However, very rarely (INVENTORYSPRITES.TXT & ESIDNEY.TXT), the style ";//" or "; //" is used.
            // So...let's just ignore any line starting with a semicolon.
            if(entry.key[0] == ';') { continue; }

            // We should not have any duplicate keys.
            #if defined(DEBUG)
            assert(mKeyToText.find(entry.key) == mKeyToText.end());
            #endif

            mKeyToText[entry.key] = entry.value;
        }
    }

    // Done with text asset.
    delete textFile;
}

std::string Localizer::GetText(const std::string& key) const
{
    // Return localized text, if we have it.
    auto it = mKeyToText.find(key);
    if(it != mKeyToText.end())
    {
        return it->second;
    }
    
    // Return the key back if no localization exists.
    return key;
}
