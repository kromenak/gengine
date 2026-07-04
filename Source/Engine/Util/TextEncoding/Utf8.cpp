#include "TextEncode.h"

#include "Platform.h"

#if defined(PLATFORM_WINDOWS)
#include <Windows.h>
#endif

void Utf8::Enable()
{
    #if defined(PLATFORM_WINDOWS)
    //TODO: On Windows 10, we could check/assert that we're using the ACP code page.

    // Allows the output console to display UTF-8 encoded text.
    SetConsoleOutputCP(CP_UTF8);

    // Allows any input from the console to also be UTF-8 encoded.
    SetConsoleCP(CP_UTF8);
    #endif
}

int Utf8::GetCodePointByteCount(uint32_t codePoint)
{
    if(codePoint < 0x80)
    {
        return 1;
    }
    if(codePoint < 0x800)
    {
        return 2;
    }
    return 3;
}

int Utf8::CodePointToUtf8(uint32_t codePoint, uint8_t* bytes)
{
    if(codePoint < 0x80)
    {
        bytes[0] = codePoint;
        return 1;
    }
    else if(codePoint < 0x800) // 2-byte UTF-8 sequence
    {
        // 11 bits of data total.
        // First byte must start with binary 110 (0xC0) followed by 5 bits of of the code point.
        // Second byte must start with binary 10 (0x80) followed by 6 bits of the code point.
        bytes[0] = static_cast<uint8_t>(0xC0 | (codePoint >> 6));
        bytes[1] = static_cast<uint8_t>(0x80 | (codePoint & 0x3F));
        return 2;
    }
    else // 3-byte UTF-8 sequence
    {
        // 16 bits of data total.
        // First byte must start with binary 1110 (0xE0) followed by 4 bits of of the code point.
        // Second byte must start with binary 10 (0x80) followed by 6 bits of the code point.
        // Third byte must start with binary 10 (0x80) followed by 6 bits of the code point.
        bytes[0] = static_cast<uint8_t>(0xE0 | (codePoint >> 12));
        bytes[1] = static_cast<uint8_t>(0x80 | ((codePoint >> 6) & 0x3F));
        bytes[2] = static_cast<uint8_t>(0x80 | (codePoint & 0x3F));
        return 3;
    }
}

std::string Utf8::CodePointToUtf8(uint32_t codePoint)
{
    std::string utf8String;
    utf8::append(codePoint, std::back_inserter(utf8String));
    return utf8String;
}

uint32_t Utf8::Utf8ToCodePoint(const std::string& utf8Character)
{
    // Return null code point if string is empty.
    if(utf8Character.empty()) { return 0; }

    // Grab the first unicode character from the string and return its codepoint.
    // We're assuming here that the passed in string is a single UTF-8 character.
    auto it = utf8Character.begin();
    return utf8::next(it, utf8Character.end());
}

size_t Utf8::CharacterIndexToByteIndex(const std::string& string, size_t characterIndex)
{
    // Use utf8 library to advance x characters into the unicode string.
    auto it = string.begin();
    utf8::advance(it, characterIndex, string.end());

    // Use std::distance to get the byte offset at that character index.
    return std::distance(string.begin(), it);
}

size_t Utf8::CharacterCountToByteCount(const std::string& string, size_t characterIndex, size_t characterCount)
{
    return CharacterIndexToByteIndex(string, characterIndex + characterCount) - CharacterIndexToByteIndex(string, characterIndex);
}

std::string Utf8::Substring(const std::string& string, size_t startCharacterIndex, size_t characterCount)
{
    if(characterCount == std::string::npos)
    {
        return string.substr(CharacterIndexToByteIndex(string, startCharacterIndex));
    }
    else
    {
        return string.substr(CharacterIndexToByteIndex(string, startCharacterIndex), CharacterCountToByteCount(string, startCharacterIndex, characterCount));
    }
}

void Utf8::Insert(std::string& string, size_t characterIndex, const std::string& toInsert)
{
    // Insert the new string at that byte index.
    string.insert(CharacterIndexToByteIndex(string, characterIndex), toInsert);
}

void Utf8::Insert(std::string& string, size_t characterIndex, uint32_t codePoint)
{
    // Convert code point to a UTF-8 string.
    std::string toInsert;
    utf8::append(codePoint, std::back_inserter(toInsert));

    // Insert like a normal string.
    Insert(string, characterIndex, toInsert);
}

void Utf8::Erase(std::string& string, size_t characterIndex, size_t characterCount)
{
    string.erase(CharacterIndexToByteIndex(string, characterIndex), CharacterCountToByteCount(string, characterIndex, characterCount));
}

void Utf8::PopBack(std::string& string)
{
    auto it = string.end();
    utf8::prior(it, string.begin());
    string.erase(it, string.end());
}
