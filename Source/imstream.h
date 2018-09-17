//
// imstream.h
//
// Clark Kromenaker
//
// A "memory stream" that can be used to feed an array of byte data
// to a reader to be read as though it were a file stream.
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
