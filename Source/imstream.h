//
//  imstream.h
//  GEngine
//
//  Created by Clark Kromenaker on 8/26/17.
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
