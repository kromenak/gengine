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
    Localizer() = default;
    explicit Localizer(const std::string& fileName, const std::string& sectionName = "");

    void Load(const std::string& fileName, const std::string& sectionName = "");

    //TODO: Change Locale
    
    std::string GetText(const std::string& key) const;
    
private:
    std::string_map_ci<std::string> mKeyToText;
};

// It's possible to create many Localizer instances, but there is also one "main" localizer.
extern Localizer gLocalizer;