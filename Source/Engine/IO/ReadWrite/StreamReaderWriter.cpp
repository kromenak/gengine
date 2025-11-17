#include "StreamReaderWriter.h"

void StreamReader::Seek(uint32_t position)
{
    // It's possible we've hit EOF, especially if we're jumping around a lot.
    // If we are trying to seek on an EOF stream, clear the error flags and do the seek.
    if(!mStream->good() && mStream->eof())
    {
        mStream->clear();
    }
    mStream->seekg(static_cast<std::streamoff>(position), std::ios::beg);
}

void StreamReader::Skip(uint32_t count)
{
    // Don't clear eof bit here, since skip is always a "move forward" action.
    mStream->seekg(static_cast<std::streamoff>(count), std::ios::cur);
}

uint32_t StreamReader::GetPosition() const
{
    // tellg may return -1 if the stream is in error state or position can't be determined.
    // I'm not sure if this would cause problems, but we'll just return 0 in this case (for now).
    std::streampos pos = mStream->tellg();
    if(pos < 0)
    {
        pos = 0;
    }
    return static_cast<uint32_t>(pos);
}

uint64_t StreamReader::Read(char* buffer, uint64_t bufferSize)
{
    mStream->read(buffer, bufferSize);
    return mStream->gcount();
}

void StreamWriter::Seek(uint32_t position)
{
    // It's possible we've hit EOF, especially if we're jumping around a lot.
    // If we are trying to seek on an EOF stream, clear the error flags and do the seek.
    if(!mStream->good() && mStream->eof())
    {
        mStream->clear();
    }
    mStream->seekp(static_cast<std::streamoff>(position), std::ios::beg);
}

void StreamWriter::Skip(uint32_t count)
{
    // Don't clear eof bit here, since skip is always a "move forward" action.
    mStream->seekp(count, std::ios::cur);
}

uint32_t StreamWriter::GetPosition() const
{
    // tellp may return -1 if the stream is in error state or position can't be determined.
    // I'm not sure if this would cause problems, but we'll just return 0 in this case (for now).
    std::streampos pos = mStream->tellp();
    if(pos < 0)
    {
        pos = 0;
    }
    return static_cast<uint32_t>(pos);
}

void StreamWriter::Write(const char* buffer, uint64_t size)
{
    mStream->write(buffer, size);
}
