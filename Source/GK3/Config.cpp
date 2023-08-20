#include "Config.h"

#include "StringUtil.h"
#include "TextWriter.h"

void Config::Load(uint8_t* data, uint32_t dataLength)
{
    // Read in each section and store it.
    IniParser parser(data, dataLength);
    parser.SetMultipleKeyValuePairsPerLine(false);

    IniSection section;
    while(parser.ReadNextSection(section))
    {
        mConfig[section.name] = section.GetAsMap();
    }
}

void Config::Save(const std::string& path)
{
    TextWriter writer(path.c_str());

    // Write out each section.
    for(auto& section : mConfig)
    {
        writer.WriteLine("[" + section.first + "]");
        for(auto& line : section.second)
        {
            writer.WriteLine(line.first + " = " + line.second.value);
        }

        // Put an empty line between sections.
        writer.WriteLine("");
    }
}

bool Config::HasKey(const std::string& key)
{
    IniKeyValue keyValue;
    return GetKeyValue(key, keyValue);
}

bool Config::HasKey(const std::string& section, const std::string& key)
{
    IniKeyValue keyValue;
    return GetKeyValue(section, key, keyValue);
}

std::string Config::GetString(const std::string& key, const std::string& defaultValue)
{
    IniKeyValue keyValue;
    if(GetKeyValue(key, keyValue))
    {
        return keyValue.value;
    }
    return defaultValue;
}

std::string Config::GetString(const std::string& section, const std::string& key, const std::string& defaultValue)
{
    IniKeyValue keyValue;
    if(GetKeyValue(section, key, keyValue))
    {
        return keyValue.value;
    }
    return defaultValue;
}

int Config::GetInt(const std::string& key, int defaultValue)
{
    IniKeyValue keyValue;
    if(GetKeyValue(key, keyValue))
    {
        return keyValue.GetValueAsInt();
    }
    return defaultValue;
}

int Config::GetInt(const std::string& section, const std::string& key, int defaultValue)
{
    IniKeyValue keyValue;
    if(GetKeyValue(section, key, keyValue))
    {
        return keyValue.GetValueAsInt();
    }
    return defaultValue;
}

float Config::GetFloat(const std::string& key, float defaultValue)
{
    IniKeyValue keyValue;
    if(GetKeyValue(key, keyValue))
    {
        return keyValue.GetValueAsFloat();
    }
    return defaultValue;
}

float Config::GetFloat(const std::string& section, const std::string& key, float defaultValue)
{
    IniKeyValue keyValue;
    if(GetKeyValue(section, key, keyValue))
    {
        return keyValue.GetValueAsFloat();
    }
    return defaultValue;
}

bool Config::GetBool(const std::string& key, bool defaultValue)
{
    IniKeyValue keyValue;
    if(GetKeyValue(key, keyValue))
    {
        return keyValue.GetValueAsBool();
    }
    return defaultValue;
}

bool Config::GetBool(const std::string& section, const std::string& key, bool defaultValue)
{
    IniKeyValue keyValue;
    if(GetKeyValue(section, key, keyValue))
    {
        return keyValue.GetValueAsBool();
    }
    return defaultValue;
}

void Config::Set(const std::string& section, const std::string& key, const std::string& value)
{
    mConfig[section][key].value = value;
}

void Config::Set(const std::string& section, const std::string& key, int value)
{
    mConfig[section][key].value = std::to_string(value);
}

void Config::Set(const std::string& section, const std::string& key, float value)
{
    mConfig[section][key].value = std::to_string(value);
}

void Config::Set(const std::string& section, const std::string& key, bool value)
{
    mConfig[section][key].value = value ? "true" : "false";
}

bool Config::GetKeyValue(const std::string& key, IniKeyValue& outKeyValue)
{
    // Since no section was specified, we'll iterate each section until we find the key.
    // This means if there are identical keys in different sections, the first will be returned.
    for(auto& section : mConfig)
    {
        for(auto& line : section.second)
        {
            if(StringUtil::EqualsIgnoreCase(line.first, key))
            {
                outKeyValue = line.second;
                return true;
            }
        }
    }

    // Did not find the key in the config!
    return false;
}

bool Config::GetKeyValue(const std::string& sectionName, const std::string& key, IniKeyValue& outKeyValue)
{
    // First, find the section.
    auto sectionIt = mConfig.find(sectionName);
    if(sectionIt == mConfig.end())
    {
        return false;
    }

    // Next, find the key.
    auto keyIt = sectionIt->second.find(key);
    if(keyIt == sectionIt->second.end())
    {
        return false;
    }

    // Found it!
    outKeyValue = keyIt->second;
    return true;
}
