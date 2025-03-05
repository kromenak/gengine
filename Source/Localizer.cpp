#include "Localizer.h"

#include <cassert>

#include "AssetManager.h"
#include "Config.h"
#include "IniParser.h"
#include "TextAsset.h"

Localizer gLocalizer;

/*static*/ const std::string& Localizer::GetLanguagePrefix()
{
    static std::string languagePrefix;
    if(languagePrefix.empty())
    {
        // If a language prefix is specified in GK3.ini, it will override anything else.
        Config* config = gAssetManager.LoadConfig("GK3.ini");
        if(config != nullptr && config->HasKey("Localization", "Locale"))
        {
            languagePrefix = config->GetString("Localization", "Locale", "E");
            printf("Language preference set to %s (from GK3.ini)\n", languagePrefix.c_str());
        }
        else
        {
            // Default to English.
            languagePrefix = "E";
        }
    }
    return languagePrefix;
}

Localizer::Localizer(const std::string& fileName, const std::string& sectionName)
{
    Load(fileName, sectionName);
}

void Localizer::Load(const std::string& fileName, const std::string& sectionName)
{
    // We assume the passed in filename does not have the language prefix attached yet.
    // Add the language prefix based on current game options.
    std::string localeFileName = GetLanguagePrefix() + fileName;

    // GK3 translations are stored in INI files with various sections.
    // Sometimes, we don't care about the sections - we just load the entire file into a single localizer.
    // However, sometimes sections have duplicate keys - in that case, it's better to treat each section as its own localizer.
    // NOTE: SIDNEY.TXT and SIDNEYEMAIL.TXT are always prefixed with en E even in other languages
    TextAsset* textFile = gAssetManager.LoadText(GetLanguagePrefix() + fileName, AssetScope::Manual);
    if(textFile == nullptr)
    {
        printf("Failed to load %s%s - falling back on English (E%s).\n", GetLanguagePrefix().c_str(), fileName.c_str(), fileName.c_str());
        textFile = gAssetManager.LoadText("E" + fileName, AssetScope::Manual);
        if(textFile == nullptr)
        {
            printf("Failed to load localization text file %s! No localized text will be loaded.\n", fileName.c_str());
            return;
        }
    }

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