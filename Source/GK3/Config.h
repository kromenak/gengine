//
// Clark Kromenaker
//
// A "Config" file is really just an INI file consisting of sections with arbitrary key/value pairs.
// The purpose is to define various "configurations" required by the game.
//
// These assets usually have a .CFG extension, but any INI-style text document can also be loaded.
//
#pragma once
#include "Asset.h"

#include <string>
#include <unordered_map>

#include "IniParser.h"

class Config : public Asset
{
public:
    Config(const std::string& name, AssetScope scope) : Asset(name, scope) { }
    void Load(uint8_t* data, uint32_t dataLength);

    void Save(const std::string& path);

    bool HasKey(const std::string& key);
    bool HasKey(const std::string& section, const std::string& key);

    std::string GetString(const std::string& key, const std::string& defaultValue = "");
    std::string GetString(const std::string& section, const std::string& key, const std::string& defaultValue = "");

    int GetInt(const std::string& key, int defaultValue = 0);
    int GetInt(const std::string& section, const std::string& key, int defaultValue = 0);

    float GetFloat(const std::string& key, float defaultValue = 0.0f);
    float GetFloat(const std::string& section, const std::string& key, float defaultValue = 0.0f);

    bool GetBool(const std::string& key, bool defaultValue = false);
    bool GetBool(const std::string& section, const std::string& key, bool defaultValue = false);

    void Set(const std::string& section, const std::string& key, const std::string& value);
    void Set(const std::string& section, const std::string& key, int value);
    void Set(const std::string& section, const std::string& key, float value);
    void Set(const std::string& section, const std::string& key, bool value);

private:
    // A map of the contents of the INI file.
    // The first key is the section name - delimited by [ ] (e.g. [GENERAL]).
    // The second key is the key part of a key/value pair on each line.
    std::unordered_map<std::string, std::unordered_map<std::string, IniKeyValue>> mConfig;

    bool GetKeyValue(const std::string& key, IniKeyValue& outKeyValue);
    bool GetKeyValue(const std::string& sectionName, const std::string& key, IniKeyValue& outKeyValue);
};