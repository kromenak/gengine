#include "DataHelper.h"

#include <vector>

#include "AssetManager.h"
#include "Config.h"
#include "FileSystem.h"
#include "Log.h"
#include "StringUtil.h"
#include "TextAsset.h"

namespace
{
    // All data directories that have been detected.
    std::vector<DataDirectory> dataDirectories;

    bool ParseDataDirectory(const std::string& path, DataDirectory& dataDirectory)
    {
        if(!Directory::Exists(path))
        {
            Logf("Data directory %s doesn't exist - ignoring.", path.c_str());
            return false;
        }

        // Since this code runs very early in program execution, we can't use the global AssetManager.
        // And it's probably less error-prone to use a local instance anyway!
        // Create an AssetManager that can load assets from this path.
        AssetManager assetManager;
        assetManager.AddSearchPath(path);

        // Make sure at least core.brn exists. If not, this is not truly a data directory.
        // We also need this later on to auto-detect which locale this is.
        if(!assetManager.LoadAssetArchive("core.brn"))
        {
            Logf("Data directory %s appears to be missing required barn files - ignoring.", path.c_str());
            return false;
        }

        // As a safeguard for custom localizations and mods, allow the Data folder to provide a CFG file explaining its configuration.
        // Rather than auto-detecting the locale, this file will just explicitly specify such things.
        Config* dataConfig = assetManager.LoadAsset<Config>("Data.cfg", AssetScope::Manual);
        if(dataConfig != nullptr)
        {
            dataDirectory.path = path;
            dataDirectory.localeName = dataConfig->GetString("LocaleName", "Unknown");
            std::string localePrefix = dataConfig->GetString("LocalePrefix", "E");
            dataDirectory.localePrefix = !localePrefix.empty() ? localePrefix[0] : 'E';

            Logf("Found data directory %s with configured locale %s (%c).", path.c_str(), dataDirectory.localeName.c_str(), dataDirectory.localePrefix);
            delete dataConfig;
            return true;
        }

        // But usually, we'll have to auto-detect the locale of the data directory.
        // Start with all known official localizations that actually renamed the ESTRINGS.TXT file.
        // For example, in French this file is called FSTRINGS, in Italian ISTRINGS, etc.
        std::pair<char, std::string> knownLocalizations[] = {
            { 'F', "fr" },
            { 'I', "it" },
            { 'G', "de" },
            { 'S', "es" }
        };
        for(const auto& pair : knownLocalizations)
        {
            // If the asset exists with the prefix char, then this data directory must be for that locale.
            TextAsset* textAsset = assetManager.LoadAsset<TextAsset>(pair.first + std::string("STRINGS.TXT"), AssetScope::Manual);
            if(textAsset != nullptr)
            {
                dataDirectory.path = path;
                dataDirectory.localeName = pair.second;
                dataDirectory.localePrefix = pair.first;

                Logf("Found data directory %s with auto-detected locale %s (%c).", path.c_str(), pair.second.c_str(), pair.first);
                delete textAsset;
                return true;
            }
        }

        // Some localizations did not bother to rename the asset files. ESTRINGS.TXT is used for English, but also for a few other localizations.
        // In this case, we can detect which localization by querying the contents of the ESTRINGS.TXT file.
        TextAsset* textAsset = assetManager.LoadAsset<TextAsset>("ESTRINGS.TXT", AssetScope::Manual);
        if(textAsset != nullptr)
        {
            // As it turns out, all known localizations have a different length for this file. So this could be an efficient way to check locale.
            //TODO: We can also read text and check a specific key, if this is not reliable enough for some reason.
            std::string localeName;
            if(textAsset->GetTextLength() == 18460)
            {
                localeName = "en";
            }
            else if(textAsset->GetTextLength() == 19847)
            {
                localeName = "pt";
            }
            else if(textAsset->GetTextLength() == 19267)
            {
                localeName = "ru";
            }
            //TODO: Polish?

            // If we couldn't detect the locale, just use an "unknown" placeholder.
            if(localeName.empty())
            {
                localeName = "un";
            }

            dataDirectory.path = path;
            dataDirectory.localeName = localeName;
            dataDirectory.localePrefix = 'E';

            Logf("Found data directory %s with auto-detected locale %s (E).", path.c_str(), localeName.c_str());
            delete textAsset;
            return true;
        }

        // Worst case, not only does this not seem to be a known locale, it doesn't even seem to be a valid Data directory.
        // Just ignore it I guess!
        Logf("Data directory %s appears to be missing important files - ignoring.", path.c_str());
        return false;
    }

    bool GetOrCreateDataDirectory(const std::string& path, DataDirectory& dataDirectory)
    {
        // Get existing if we already parsed this one.
        for(const DataDirectory& dataDir : dataDirectories)
        {
            if(StringUtil::EqualsIgnoreCase(dataDir.path, path))
            {
                dataDirectory = dataDir;
                return true;
            }
        }

        // Otherwise, parse it and return it!
        return ParseDataDirectory(path, dataDirectory);
    }

    void DetectDataDirectories()
    {
        // Only do this if we haven't already detected directories.
        if(!dataDirectories.empty()) { return; }

        // Get all directories starting with "Data" in the executable directory.
        // Iterate and parse each one.
        std::vector<std::string> dataDirectoryPaths = Directory::List(".", FILETYPE_DIRECTORY, "Data*");
        for(const std::string& dataDirectoryPath : dataDirectoryPaths)
        {
            DataDirectory dataDirectory;
            if(ParseDataDirectory(dataDirectoryPath, dataDirectory))
            {
                dataDirectories.push_back(dataDirectory);
            }
        }
    }
}

bool DataHelper::GetDataDirectoryToUse(DataDirectory& dataDirectory)
{
    // Attempt to load GK3.ini config so we can check for data directory and locale preferences.
    AssetManager assetManager;
    assetManager.AddSearchPath("");
    Config* config = assetManager.LoadAsset<Config>("GK3.ini", AssetScope::Manual);
    if(config != nullptr)
    {
        // If a Data Directory preference is specified, try to use that directly.
        std::string dataDirectoryPreference = config->GetString("Resources", "Data Directory", "");
        if(!dataDirectoryPreference.empty())
        {
            if(GetOrCreateDataDirectory(dataDirectoryPreference, dataDirectory))
            {
                Logf("Using data directory %s due to Data Directory preference in GK3.ini", dataDirectory.path.c_str());
                return true;
            }
        }

        // If no data directory preference was specified, or if it was invalid, we'll need to detect our available options.
        DetectDataDirectories();

        // Attempt to choose a Data directory based on locale preference, if one exists.
        std::string localePreference = config->GetString("Localization", "Locale", "");
        if(!localePreference.empty())
        {
            for(const DataDirectory& dataDir : dataDirectories)
            {
                if(StringUtil::EqualsIgnoreCase(dataDir.localeName, localePreference))
                {
                    dataDirectory = dataDir;
                    Logf("Using data directory %s due to Locale preference in GK3.ini", dataDirectory.path.c_str());
                    return true;
                }
            }
        }
        delete config;
    }

    // Without any preference or config value, let's try to use Data.
    if(GetOrCreateDataDirectory("Data", dataDirectory))
    {
        Log("Using data directory Data.");
        return true;
    }

    // If no "Data" directory, just try to use ANY directory that begins with "Data".
    DetectDataDirectories();
    if(!dataDirectories.empty())
    {
        dataDirectory = dataDirectories[0];
        Logf("Using data directory %s because it's the first one we found.", dataDirectory.path.c_str());
        return true;
    }

    // Wow, couldn't find anything.
    return false;
}
