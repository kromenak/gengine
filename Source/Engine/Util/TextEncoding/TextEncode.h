//
// Clark Kromenaker
//
// The TextEncode namespace provides helpers for converting to and from UTF-8.
// This isn't comprehensive - a converter is only added when it's needed.
//
// The Utf8 namespace contains helpers to aid in working with UTF-8 strings and Unicode code points.
//
#pragma once
#include <string>

#include <utf8.h>

namespace TextEncode
{
    // UTF-16
    std::wstring Utf8ToUtf16(const std::string& utf8Str);
    std::string Utf16ToUtf8(const std::wstring& utf16Str);

    // Extended ASCII Code Page 1251 - Cyrillic (Russian, Ukrainian, Belarusian, etc)
    std::string Cp1251ToUtf8(const std::string& string);
    uint8_t* Cp1251ToUtf8(uint8_t* inBuffer, uint32_t inBufferSize, uint32_t& outBufferSize);

    // Extended ASCII Code Page 1252 - Western European
    std::string Cp1252ToUtf8(const std::string& string);
    uint8_t* Cp1252ToUtf8(uint8_t* inBuffer, uint32_t inBufferSize, uint32_t& outBufferSize);
}

namespace Utf8
{
    // The white square Unicode code point is used when invalid or unrenderable characters are encountered.
    const uint32_t kWhiteSquareCodePoint = 9633;

    // Enable UTF-8 on this Platform
    void Enable();

    // Code Point Support
    int GetCodePointByteCount(uint32_t codePoint);
    int CodePointToUtf8(uint32_t codePoint, uint8_t* outBytes);
    std::string CodePointToUtf8(uint32_t codePoint);
    uint32_t Utf8ToCodePoint(const std::string& utf8Character);

    // Code Point Character Index/Count to Byte Index/Count
    size_t CharacterIndexToByteIndex(const std::string& string, size_t characterIndex);
    size_t CharacterCountToByteCount(const std::string& string, size_t characterIndex, size_t characterCount);

    // UTF-8 String Manipulation
    std::string Substring(const std::string& string, size_t startCharacterIndex, size_t characterCount = std::string::npos);
    void Insert(std::string& string, size_t characterIndex, const std::string& toInsert);
    void Insert(std::string& string, size_t characterIndex, uint32_t codePoint);
    void Erase(std::string& string, size_t characterIndex, size_t characterCount);
    void PopBack(std::string& string);
}