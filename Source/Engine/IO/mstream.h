//
// Clark Kromenaker
//
// A "memory stream", for reading/writing memory via a stream.
// Like an "fstream," but reads/writes data from/to byte array rather than a file.
//
#pragma once
#include <cstdint>
#include <istream>
#include <ostream>
#include <streambuf>

// A stream buffer that operates on an arbitrary piece of memory.
// C++ provides stream buffers for files & strings, but not byte arrays (as far as I can tell).
class membuf : public std::streambuf
{
public:
    membuf(char* data, uint32_t length, std::ios_base::openmode which);

protected:
    std::streampos seekoff(std::streamoff off, std::ios_base::seekdir way,
                           std::ios_base::openmode which = std::ios_base::in | std::ios_base::out) override;
    std::streampos seekpos(std::streampos pos,
                           std::ios_base::openmode which = std::ios_base::in | std::ios_base::out) override;
};

// Input stream that uses memory buffer.
class imstream : public std::istream
{
public:
    imstream(const char* data, uint32_t length);

private:
    membuf buffer;
};

// Output stream that uses memory buffer.
class omstream : public std::ostream
{
public:
    omstream(char* data, uint32_t length);

private:
    membuf buffer;
};
