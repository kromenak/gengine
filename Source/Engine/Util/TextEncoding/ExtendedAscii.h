//
// Clark Kromenaker
//
// The logic for converting Extended ASCII code pages to UTF-8 is similar for each code page.
// This header defines the shared logic to avoid duplicating code.
//
#pragma once
#include <cstdint>
#include <string>

#include "BinaryWriter.h"
#include "TextEncode.h"

namespace ExtendedAscii
{
    inline uint32_t ToCodePoint(uint8_t extendedAsciiValue, const uint32_t* codePointMap)
    {
        if(extendedAsciiValue < 0x80)
        {
            return extendedAsciiValue;
        }
        else
        {
            return codePointMap[extendedAsciiValue - 0x80];
        }
    }

    inline uint8_t* ToUtf8(uint8_t* inBuffer, uint32_t inBufferSize, uint32_t& outBufferSize, const uint32_t* codePointMap)
    {
        // Calculate how many bytes we need to represent the string in UTF-8.
        uint32_t byteCountForUtf8 = 0;
        for(uint32_t i = 0; i < inBufferSize; ++i)
        {
            byteCountForUtf8 += Utf8::GetCodePointByteCount(ToCodePoint(inBuffer[i], codePointMap));
        }

        // Allocate an output buffer.
        uint8_t* outBuffer = new uint8_t[byteCountForUtf8];

        // Write bytes to the output buffer corresponding to UTF-8 version of input buffer.
        BinaryWriter writer(outBuffer, byteCountForUtf8);
        uint8_t bytes[3] = { 0 };
        for(uint32_t i = 0; i < inBufferSize; ++i)
        {
            int byteCount = Utf8::CodePointToUtf8(ToCodePoint(inBuffer[i], codePointMap), bytes);
            writer.Write(bytes, byteCount);
        }

        // Record output buffer size and return output buffer.
        outBufferSize = byteCountForUtf8;
        return outBuffer;
    }

    inline std::string ToUtf8String(const std::string& extendedAsciiString, const uint32_t* codePointMap)
    {
        // Extended ASCII uses 1 byte per character but UTF-8 uses 1-4 bytes per character.
        // So a reasonable middleground is to assume the UTF-8 version will be somewhat larger than the extended ascii version.
        std::string output;
        output.reserve(extendedAsciiString.size() * 2);

        // Iterate the incoming CP1251 string. For each character, convert to UTF-8 bytes and append to output string.
        uint8_t bytes[3] = { 0 };
        for(uint8_t c : extendedAsciiString)
        {
            int byteCount = Utf8::CodePointToUtf8(ToCodePoint(c, codePointMap), bytes);
            output.append(reinterpret_cast<char*>(bytes), byteCount);
        }
        return output;
    }
}