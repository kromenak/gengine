//
// Clark Kromenaker
//
// Provides functions to aid in converting from certain text encodings to/from UTF-8.
//
// This isn't meant to be comprehensive - conversion functions are only provided if needed.
// For example, I can't immediately see a need for a "UTF-8 to CP1251" style function!
//
#pragma once
#include <string>

namespace TextEncode
{
    // Windows shenanigans
    void InitUtf8();

    // UTF-8 to/from UTF-16 wstring
    std::wstring Utf8ToUtf16(const std::string& utf8Str);
    std::string Utf16ToUtf8(const std::wstring& utf16Str);

    // Code Page 1251 - Cyrillic (Russian, Ukrainian, Belarusian, etc)
    std::string Cp1251ToUtf8(const std::string& cp1251str);
    uint8_t* Cp1251ToUtf8(uint8_t* inBuffer, uint32_t inBufferSize, uint32_t& outBufferSize);
}