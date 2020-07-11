//
// Localizer.h
//
// Clark Kromenaker
//
// Provides a mapping from localization keys to localized text.
//
#pragma once

#include <string>
#include <unordered_map>

#include "Type.h"

class Localizer
{
    TYPE_DECL_BASE();
public:
    Localizer();
    
    //TODO: Change Locale
    
    std::string GetText(const std::string& key) const;
    
private:
    std::unordered_map<std::string, std::string> mKeyToText;
};
