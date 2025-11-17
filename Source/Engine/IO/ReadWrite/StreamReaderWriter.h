//
// Clark Kromenaker
//
// Base classes for stream readers and writers.
//
#pragma once
#include <iostream>

class StreamReader
{
public:
    StreamReader(std::istream* stream, bool ownsStream = false) : mStream(stream), mOwnsStream(ownsStream) { }
    ~StreamReader() { if(mOwnsStream) { delete mStream; } }

    bool CanRead() const { return mStream->good(); } // "good" returns true when fail/bad/eof bits are all false
    bool EndOfFile() const { return mStream->eof(); }

    void Seek(uint32_t position);
    void Skip(uint32_t count);
    uint32_t GetPosition() const;

    uint64_t Read(char* buffer, uint64_t bufferSize);

protected:
    // The stream being read.
    std::istream* mStream = nullptr;

private:
    // If true, we own the stream object, so we should delete it in destructor.
    bool mOwnsStream = false;
};

class StreamWriter
{
public:
    StreamWriter(std::ostream* stream, bool ownsStream = false) : mStream(stream), mOwnsStream(ownsStream) { }
    ~StreamWriter() { if(mOwnsStream) { delete mStream; } }

    bool CanWrite() const { return mStream->good(); } // "good" returns true when fail/bad/eof bits are all false

    void Seek(uint32_t position);
    void Skip(uint32_t count);
    uint32_t GetPosition() const;

    void Write(const char* buffer, uint64_t size);
    void Flush() { mStream->flush(); }

protected:
    // The stream being written.
    std::ostream* mStream = nullptr;

private:
    // If true, we own the stream object, so we should delete it in destructor.
    bool mOwnsStream = false;
};

