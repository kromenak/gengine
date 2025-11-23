#include "Ini.h"

#include "StringUtil.h"

float IniKeyValue::GetValueAsFloat() const
{
    return StringUtil::ToFloat(value);
}

int IniKeyValue::GetValueAsInt() const
{
    return StringUtil::ToInt(value);
}

bool IniKeyValue::GetValueAsBool() const
{
    return StringUtil::ToBool(value);
}

Vector2 IniKeyValue::GetValueAsVector2() const
{
    return Vector2::Parse(value);
}

Vector3 IniKeyValue::GetValueAsVector3() const
{
    return Vector3::Parse(value);
}

Vector4 IniKeyValue::GetValueAsVector4() const
{
    return Vector4::Parse(value);
}

Color32 IniKeyValue::GetValueAsColor32() const
{
    // Assume string form of R/G/B
    std::size_t firstSlashIndex = value.find('/');
    if(firstSlashIndex == std::string::npos)
    {
        return Color32::Black;
    }
    std::size_t secondSlashIndex = value.find('/', firstSlashIndex + 1);
    if(secondSlashIndex == std::string::npos)
    {
        return Color32::Black;
    }

    // Split at slashes.
    std::string firstNum = value.substr(0, firstSlashIndex);
    std::string secondNum = value.substr(firstSlashIndex + 1, secondSlashIndex - firstSlashIndex - 1);
    std::string thirdNum = value.substr(secondSlashIndex + 1, std::string::npos);

    // Convert to number and return.
    return Color32(std::stoi(firstNum), std::stoi(secondNum), std::stoi(thirdNum));
}

Rect IniKeyValue::GetValueAsRect() const
{
    // We assume the string form of {4.23, 5.23, 10.04, 5.23}
    // Values are in order of x1, z1, x2, z2 for two points.
    // First, let's get rid of the braces.
    std::string noBraces = value;
    if(noBraces[0] == '{' && noBraces[noBraces.size() - 1] == '}')
    {
        noBraces = value.substr(1, value.length() - 2);
    }

    // Split into 4 elements, divided by commas.
    // In at least one instance, errant commas can trip this up, so discard empty elements.
    std::vector<std::string> elements = StringUtil::Split(noBraces, ',', true);
    if(elements.size() < 4)
    {
        return Rect();
    }

    // Convert to numbers and return.
    Vector2 p1(std::stof(elements[0]), std::stof(elements[1]));
    Vector2 p2(std::stof(elements[2]), std::stof(elements[3]));
    return Rect(p1, p2);
}

std::unordered_map<std::string, IniKeyValue> IniSection::GetAsMap() const
{
    std::unordered_map<std::string, IniKeyValue> map;
    for(auto& line : lines)
    {
        for(auto& entry : line.entries)
        {
            map[entry.key] = entry;
        }
    }
    return map;
}