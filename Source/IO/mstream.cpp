#include "mstream.h"

membuf::membuf(char* data, uint32_t length, std::ios_base::openmode which)
{
    // setg is used for input (reading)
    // setp is used for output (writing)
    if((which & std::ios_base::out) != 0)
    {
        setp(data, data + length);
    }
    else if((which & std::ios_base::in) != 0)
    {
        setg(data, data, data + length);
    }
}

std::streampos membuf::seekoff(std::streamoff off, std::ios_base::seekdir way, std::ios_base::openmode which)
{
    // Determine whether we're doing out or in.
    bool out = ((which & std::ios_base::out) != 0);
    bool in = ((which & std::ios_base::in) != 0);
    if(!out && !in) { return -1; }

    // Determine which pointers to use for beg/cur/end.
    char* beg = nullptr;
    char* cur = nullptr;
    char* end = nullptr;
    if(out)
    {
        beg = pbase();
        cur = pptr();
        end = epptr();
    }
    else if(in)
    {
        beg = eback();
        cur = gptr();
        end = egptr();
    }

    // Need these pointers to be valid!
    if(beg == nullptr || cur == nullptr || end == nullptr)
    {
        return -1;
    }

    // Calculate update based on way and offset.
    if(way == std::ios_base::beg)
    {
        cur = beg + off;
    }
    else if(way == std::ios_base::cur)
    {
        cur += off;
    }
    else if(way == std::ios_base::end)
    {
        cur = end - off;
    }

    // Clamp to begin/end.
    if(cur > end)
    {
        cur = end;
    }
    else if(cur < beg)
    {
        cur = beg;
    }
    
    // Calc where we are in the stream.
    std::streampos offset = cur - beg;

    // Update pointers.
    if(out)
    {
        setp(beg, end);
        pbump(offset);
    }
    else if(in)
    {
        setg(beg, cur, end);
    }

    // Return current offset from beginning.
    return offset;
}

std::streampos membuf::seekpos(std::streampos pos, std::ios_base::openmode which)
{
    // This just always goes from the beginning.
    return seekoff(pos, std::ios_base::beg, which);
}

imstream::imstream(const char* data, uint32_t length) :
    std::istream(&buffer),
    buffer(const_cast<char*>(data), length, std::ios_base::in)  // you'd expect an imstream to be const (reading only, no writing), and it is...
                                                                // but in this case, you've got to "trust us" because streambufs require non-const pointers
{

}

omstream::omstream(char* data, uint32_t length) :
    std::ostream(&buffer),
    buffer(data, length, std::ios_base::out)
{

}
