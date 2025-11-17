#include "Ini.h"

#include "StringUtil.h"

namespace
{
    std::string TrimVectorToCommaSeparatedString(const std::string& vectorString)
    {
        // Input is a string representing a 2D/3D/4D vector. May have leading/trailing cruft, like braces.
        // Example: { -20, 30, 200, 10 }
        // Example: 20, 5

        // Trim leading non-numeric values.
        // This is usually just '{', but other garbage data (from typos) exists in some cases.
        int firstNumIndex = -1;
        for(int i = 0; i < vectorString.length(); ++i)
        {
            // Also need to check '-' for symbol preceding negative numbers.
            // Also also need to check for '.' symbol for shorthand like ".42"
            if(std::isdigit(vectorString[i]) || vectorString[i] == '-' || vectorString[i] == '.')
            {
                firstNumIndex = i;
                break;
            }
        }
        if(firstNumIndex == -1) { return std::string(); }

        // Trim trailing non-numeric values. Usually just '}'.
        std::string noBraces = vectorString.substr(firstNumIndex);
        while(!noBraces.empty() && !std::isdigit(noBraces.back()))
        {
            noBraces.pop_back();
        }
        return noBraces;
    }

    void SanitizeNumberString(std::string& numberString)
    {
        // In at least one instance (Devil's Armchair files), too many negative signs exist before a number.
        while(numberString.size() >= 2 && numberString[0] == '-' && numberString[1] == '-')
        {
            numberString.erase(0, 1);
        }
    }
}

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
    // We assume the string form of {4.23, 5.23}
    // First, let's get rid of the braces so we just have comma-separated numbers.
    std::string noBraces = TrimVectorToCommaSeparatedString(value);

    // Find the comma index. If not present, this isn't the right form, so we fail.
    std::size_t commaIndex = noBraces.find(',');
    if(commaIndex == std::string::npos) { return Vector2::Zero; }

    // Split at the comma.
    std::string firstNum = noBraces.substr(0, commaIndex);
    std::string secondNum = noBraces.substr(commaIndex + 1, std::string::npos);

    // Make sure the number strings appear valid.
    SanitizeNumberString(firstNum);
    SanitizeNumberString(secondNum);

    // Convert to numbers and return.
    return Vector2(std::stof(firstNum), std::stof(secondNum));
}

Vector3 IniKeyValue::GetValueAsVector3() const
{
    // We generally assume the string form of {4.23, 5.23, 10.04}.
    // First, let's get rid of the braces so we just have comma-separated numbers.
    std::string noBraces = TrimVectorToCommaSeparatedString(value);

    // Find the two commas.
    std::size_t firstCommaIndex = noBraces.find(',');
    if(firstCommaIndex == std::string::npos) { return Vector3::Zero; }
    std::size_t secondCommaIndex = noBraces.find(',', firstCommaIndex + 1);
    if(secondCommaIndex == std::string::npos) { return Vector3::Zero; }

    // Split at commas.
    std::string firstNum = noBraces.substr(0, firstCommaIndex);
    std::string secondNum = noBraces.substr(firstCommaIndex + 1, secondCommaIndex - firstCommaIndex - 1);
    std::string thirdNum = noBraces.substr(secondCommaIndex + 1, std::string::npos);

    // Make sure the number strings appear valid.
    SanitizeNumberString(firstNum);
    SanitizeNumberString(secondNum);
    SanitizeNumberString(thirdNum);

    // Convert to numbers and return.
    return Vector3(std::stof(firstNum), std::stof(secondNum), std::stof(thirdNum));
}

Vector4 IniKeyValue::GetValueAsVector4() const
{
    // We generally assume the string form of {4.23, 5.23, 10.04, -14.3}.
    // First, let's get rid of the braces so we just have comma-separated numbers.
    std::string noBraces = TrimVectorToCommaSeparatedString(value);

    // Find the three commas.
    std::size_t firstCommaIndex = noBraces.find(',');
    if(firstCommaIndex == std::string::npos) { return Vector4::Zero; }
    std::size_t secondCommaIndex = noBraces.find(',', firstCommaIndex + 1);
    if(secondCommaIndex == std::string::npos) { return Vector4::Zero; }
    std::size_t thirdCommaIndex = noBraces.find(',', secondCommaIndex + 1);
    if(thirdCommaIndex == std::string::npos) { return Vector4::Zero; }

    // Split at commas.
    std::string firstNum = noBraces.substr(0, firstCommaIndex);
    std::string secondNum = noBraces.substr(firstCommaIndex + 1, secondCommaIndex - firstCommaIndex - 1);
    std::string thirdNum = noBraces.substr(secondCommaIndex + 1, thirdCommaIndex - secondCommaIndex - 1);
    std::string fourthNum = noBraces.substr(thirdCommaIndex + 1, std::string::npos);

    // Make sure the number strings appear valid.
    SanitizeNumberString(firstNum);
    SanitizeNumberString(secondNum);
    SanitizeNumberString(thirdNum);
    SanitizeNumberString(fourthNum);

    // Convert to numbers and return.
    return Vector4(std::stof(firstNum), std::stof(secondNum), std::stof(thirdNum), std::stof(fourthNum));
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