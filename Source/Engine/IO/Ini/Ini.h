//
// Clark Kromenaker
//
// Helper structs for the INI file format.
//
#pragma once
#include <string>
#include <unordered_map>
#include <vector>

#include "Color32.h"
#include "Rect.h"
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"

// A single key value pair in the INI file. Syntax is "key = value".
struct IniKeyValue
{
    std::string key;
    std::string value;

    float GetValueAsFloat() const;
    int GetValueAsInt() const;
    bool GetValueAsBool() const;
    Vector2 GetValueAsVector2() const;
    Vector3 GetValueAsVector3() const;
    Vector4 GetValueAsVector4() const;
    Color32 GetValueAsColor32() const;
    Rect GetValueAsRect() const;
};

// A line can consist of one or more key/value pairs.
// Usually there's only one per line, but there can optionally be multiple per line (separated by a delimeter char).
struct IniLine
{
    std::vector<IniKeyValue> entries;
};

// All INI lines exist within a section. Sections are identified with headers, formatted "[SectionName]".
// If an INI line exists before any section header, it's considered to be in the "default" section.
struct IniSection
{
    std::string name;
    std::string condition;
    std::vector<IniLine> lines;

    std::unordered_map<std::string, IniKeyValue> GetAsMap() const;
};