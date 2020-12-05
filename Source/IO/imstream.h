//
// imstream.h
//
// Clark Kromenaker
//
// An "input memory stream", for reading memory via a stream.
// Like an "ifstream," but reads in data from byte array rather than a file.
//
#pragma once
#include <istream>

#include "membuf.h"

class imstream : public std::istream
{
public:
    imstream(const char* data, unsigned int length);
    ~imstream();
};
