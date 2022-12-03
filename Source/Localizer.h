//
// Clark Kromenaker
//
// Provides a mapping from localization keys to localized text.
//
#pragma once
#include <string>
#include <unordered_map>

#include "StringUtil.h"
#include "Type.h"

class Localizer
{
    TYPE_DECL_BASE();
public:
    Localizer() = default;
    Localizer(const std::string& fileName, const std::string& sectionName = "");

    void Load(const std::string& fileName, const std::string& sectionName);

    //TODO: Change Locale
    
    std::string GetText(const std::string& key) const;
    
private:
    std::string_map_ci<std::string> mKeyToText;
};
