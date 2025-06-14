//
// Clark Kromenaker
//
// Provides a mapping from localization keys to localized text.
//
#pragma once
#include <string>
#include <unordered_map>

#include "StringUtil.h"

class Localizer
{
public:
    static const std::string& GetLanguagePrefix();

    Localizer() = default;
    explicit Localizer(const std::string& fileName, const std::string& sectionName = "");

    void Load(const std::string& fileName, const std::string& sectionName = "");

    bool HasText(const std::string& key) const;
    std::string GetText(const std::string& key) const;

private:
    // Maps a localization key to the localized text.
    std::string_map_ci<std::string> mKeyToText;
};

// It's possible to create many Localizer instances, but there is also one "main" localizer.
extern Localizer gLocalizer;
