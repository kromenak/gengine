#include "Localizer.h"

#include <cassert>

#include "Config.h"
#include "FileSystem.h"
#include "IniReader.h"
#include "Paths.h"
#include "StringUtil.h"
#include "TextAsset.h"

Localizer gLocalizer;

namespace
{
    // A prefix added to the front of certain localized assets.
    std::string localePrefix = "E";
}

void Localizer::SetLocalePrefix(const std::string& prefix)
{
    localePrefix = prefix;
}

/*static*/ const std::string& Localizer::GetLocalePrefix()
{
    return localePrefix;
}

Localizer::Localizer(const std::string& fileName, const std::string& sectionName)
{
    Load(fileName, sectionName);
}

void Localizer::Load(const std::string& fileName, const std::string& sectionName)
{
    // GK3 translations are stored in INI files with various sections.
    // Sometimes, we don't care about the sections - we just load the entire file into a single localizer.
    // However, sometimes sections have duplicate keys - in that case, it's better to treat each section as its own localizer.
    // NOTE: SIDNEY.TXT and SIDNEYEMAIL.TXT are always prefixed with an E even in other languages
    TextAsset* textFile = LoadLocalizedAsset<TextAsset>(fileName, AssetScope::Manual);
    if(textFile == nullptr)
    {
        return;
    }

    // Parse as INI file.
    // Ignore multiple key values per line b/c a comma is going to be part of the translation copy, rather than another key/value on the same line.
    IniReader parser(textFile->GetText(), textFile->GetTextLength());
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
        for(IniLine& line : section.lines)
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

            // Save key and text to the map.
            // "Unescape" the text too, since localized text is allowed to have line breaks and tab breaks and such in them.
            mKeyToText[entry.key] = StringUtil::Unescape(entry.value);
        }
    }

    // Done with text asset.
    delete textFile;
}

bool Localizer::HasText(const std::string& key) const
{
    return mKeyToText.find(key) != mKeyToText.end();
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