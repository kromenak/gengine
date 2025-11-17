//
// Clark Kromenaker
//
// Writes text to a stream.
//
#pragma once
#include "StreamReaderWriter.h"

#include <initializer_list>
#include <ostream>

class TextWriter : public StreamWriter
{
public:
    TextWriter(const char* filePath);
    TextWriter(std::ostream* stream);

    // Write text
    void Write(const char* text) { *mStream << text; }
    void Write(const std::string& text) { *mStream << text; }
    void WriteLine(const char* text) { *mStream << text << "\n"; }
    void WriteLine(const std::string& text) { *mStream << text << "\n"; }
    void WriteNewLine() { *mStream << "\n"; }

    // Write text at a list of elements
    void Write(std::initializer_list<const char*> list);
    void Write(std::initializer_list<std::string> list);
    void WriteLine(std::initializer_list<const char*> list);
    void WriteLine(std::initializer_list<std::string> list);

    // Write numeric types
    void WriteByte(uint8_t val) { *mStream << val; }
    void WriteSByte(int8_t val) { *mStream << val; }

    void WriteUShort(uint16_t val) { *mStream << val; }
    void WriteShort(int16_t val) { *mStream << val; }

    void WriteUInt(uint32_t val) { *mStream << val; }
    void WriteInt(int32_t val) { *mStream << val; }

    void WriteULong(uint64_t val) { *mStream << val; }
    void WriteLong(int64_t val) { *mStream << val; }

    void WriteFloat(float val) { *mStream << val; }
    void WriteDouble(double val) { *mStream << val; }

private:

};